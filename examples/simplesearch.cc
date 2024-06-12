/** @file
 * @brief Simple command-line search utility.
 *
 * See "quest" for a more sophisticated example.
 */
/* Copyright (C) 2007-2022 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <xapian.h>

#include <iostream>
#include <string>

#include <cstdlib> // For exit().
#include <cstring>

using namespace std;


const char* const K_DB_PATH = "index_data";
const char* const K_DOC_UNIQUE_ID = "007";

#include <random>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <vector>

std::vector<int> keys;

std::string key_prefix = "";
static int init_vars() {
  for (int i = 0; i < 1000; i++) {
    key_prefix += "1";
  }
  return 0;
}
static int k = init_vars();

void testCsInserts(int count) {
	const char* my_cs_index_name = "my_cstore_tab_col";

	{
		Xapian::WritableDatabase db(my_cs_index_name,
				Xapian::DB_CREATE_OR_OPEN);
		db.begin_transaction();

		for (size_t i = 0; i < count; i++) {
			std::string key, tag = std::to_string(i);
			std::random_device rd;
			std::mt19937 mt(rd());
			std::uniform_int_distribution<int> dist(0, 1000000000);
			int n_key = dist(mt);
			//int n_key = i;
			//int n_key = 12345;
			if (i <= 10000 && count < 20000)
				keys.push_back(n_key);
			key = key_prefix + std::to_string(n_key);

			std::string tag2;
			bool find = db.CS_get_exact_entry(key, tag2);
			db.CS_add(key, std::to_string(n_key));

			bool find2 = db.CS_get_exact_entry(key, tag2);
			assert(find2);

			if (i % 10000 == 0) {
				std::cout << "i=" << i << std::endl;
			}
		}

		db.commit_transaction();
		db.commit();
	}
}

void testCsQuerys() {
    const char* my_cs_index_name = "my_cstore_tab_col";

	{
		Xapian::Database db(my_cs_index_name);
		for (size_t i = 0; i < keys.size(); i++) {
			std::string key, tag = std::to_string(i), tag2;

			int n_key = keys[i];
			key = key_prefix + std::to_string(n_key);
			//key = std::string((char*)&n_key, sizeof(int));

			//docdata_table.add(key, tag);
			bool find = db.CS_get_exact_entry(key, tag2);
			assert(find);
			assert(tag2 == std::to_string(n_key));
		}
	}
}

#include <thread>
#include <chrono>
void testCsAPI() {
  testCsInserts(10000);
  testCsQuerys();

  std::thread t1(testCsInserts, 100000000);

  std::this_thread::sleep_for(std::chrono::milliseconds(2000));

  std::thread t2(testCsQuerys);

  t2.join();
  t1.join();

}
/// 创建索引
void createIndex() {
	std::cout << "--index start--" << std::endl;
	Xapian::WritableDatabase db(K_DB_PATH, Xapian::DB_CREATE_OR_OPEN);

	//db.CStoreTest();
	Xapian::Document doc;
	doc.add_posting("T中国", 1);
	doc.add_posting("T篮球", 1);
	doc.add_posting("T比赛", 1);
	doc.set_data("中国篮球比赛");
	doc.add_boolean_term(K_DOC_UNIQUE_ID);

	Xapian::docid innerId = db.replace_document(K_DOC_UNIQUE_ID, doc);

	std::cout << "add doc innerId=" << innerId << std::endl;

	db.commit();

	{
		Xapian::Document doc2;
		doc2.add_posting("T中国", 1);
		doc2.add_posting("T篮球", 1);
		doc2.add_posting("T比赛", 1);
		doc2.set_data("中国篮球比赛2");
		doc2.add_boolean_term("008");
		Xapian::docid innerId2 = db.replace_document("008", doc2);

		std::cout << "add doc innerId2=" << innerId2 << std::endl;

		db.commit();
	}

	std::cout << "--index finish--" << std::endl;
}

/// 检索索引
void queryIndex() {
	std::cout << "--search start--" << std::endl;
	Xapian::Query termOne = Xapian::Query("T中国");
	Xapian::Query termTwo = Xapian::Query("T比赛");
	Xapian::Query termThree = Xapian::Query("T足球");
	auto query = Xapian::Query(Xapian::Query::OP_OR,
			Xapian::Query(Xapian::Query::OP_OR, termOne, termTwo), termThree);
	std::cout << "query=" << query.get_description() << std::endl;

	Xapian::Database db(K_DB_PATH);

	Xapian::Enquire enquire(db);
	enquire.set_query(query);
	Xapian::MSet result = enquire.get_mset(0, 10);
	std::cout << "find results count=" << result.get_matches_estimated()
			<< std::endl;

	for (auto it = result.begin(); it != result.end(); ++it) {
		Xapian::Document doc = it.get_document();
		std::string data = doc.get_data();
		Xapian::weight docScoreWeight = it.get_weight();
		Xapian::percent docScorePercent = it.get_percent();

		std::cout << "doc=" << data << ",weight=" << docScoreWeight
				<< ",percent=" << docScorePercent << std::endl;
	}

	std::cout << "--search finish--" << std::endl;
}

int
main(int argc, char **argv)
try {


	testCsAPI();

	createIndex();
	queryIndex();


    // We require at least two command line arguments.
    if (argc < 3) {
	int rc = 1;
	if (argv[1]) {
	    if (strcmp(argv[1], "--version") == 0) {
		cout << "simplesearch\n";
		exit(0);
	    }
	    if (strcmp(argv[1], "--help") == 0) {
		rc = 0;
	    }
	}
	cout << "Usage: " << argv[0] << " PATH_TO_DATABASE QUERY\n";
	exit(rc);
    }

    // Open the database for searching.
    Xapian::Database db(argv[1]);

    // Start an enquire session.
    Xapian::Enquire enquire(db);

    // Combine the rest of the command line arguments with spaces between
    // them, so that simple queries don't have to be quoted at the shell
    // level.
    string query_string(argv[2]);
    argv += 3;
    while (*argv) {
	query_string += ' ';
	query_string += *argv++;
    }

    // Parse the query string to produce a Xapian::Query object.
    Xapian::QueryParser qp;
    Xapian::Stem stemmer("english");
    qp.set_stemmer(stemmer);
    qp.set_database(db);
    qp.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    Xapian::Query query = qp.parse_query(query_string);
    cout << "Parsed query is: " << query.get_description() << '\n';

    // Find the top 10 results for the query.
    enquire.set_query(query);
    Xapian::MSet matches = enquire.get_mset(0, 10);

    // Display the results.
    cout << matches.get_matches_estimated() << " results found.\n";
    cout << "Matches 1-" << matches.size() << ":\n\n";

    for (Xapian::MSetIterator i = matches.begin(); i != matches.end(); ++i) {
	cout << i.get_rank() + 1 << ": " << i.get_weight() << " docid=" << *i
	     << " [" << i.get_document().get_data() << "]\n\n";
    }
} catch (const Xapian::Error &e) {
    cout << e.get_description() << '\n';
    exit(1);
}
