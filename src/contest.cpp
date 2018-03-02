#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include "mysqlwrapper.h"
#include "svb_slda.h"

using namespace std;

vector<string> split(const string &str, char sep)
{
    vector<string> v;
    stringstream ss(str);
    string buffer;
    while( getline(ss, buffer, sep) ) {
        v.push_back(buffer);
    }
    return v;
}

int compare_double(const void *a, const void *b)
{
    return *(double*)b - *(double*)a;
}

int main(void) {
	MySqlWrapper2 database("localhost", "root", "password", "nlp_dataset");
	//string sql = "SELECT superviced, comment_text FROM train LIMIT 100000";
	string sql = "SELECT 1, content FROM wikipedia_en LIMIT 500000";
	unique_ptr<MYSQL_RES, decltype(&mysql_free_result)> db_result(database.query(sql), mysql_free_result);
	int latest_vocab_id = 0;
	map<string, int> vocab_ids;
	map<string, int> vocab_counters;
	map<int, string> id2word;
	MYSQL_ROW row;
	int d = 0;
	int n_document = (int)mysql_num_rows(db_result.get());
	int **word_indexes = new int*[n_document];
	unsigned short **word_counts = new unsigned short*[n_document];
	int *n_word_type_each_doc = new int[n_document];
	int *n_word_each_doc = new int[n_document];
	int *sample_y = new int[n_document];
	int n_word_type;
	unsigned long long n_all_word=0;

	while ((row = database.getRow())) {
		sample_y[d] = atoi(row[0]);
		map<int, unsigned short> vocab_ids_doc_all;
		vector<string> token = split(row[1], ' ');
		for (string word : token) {
			if (vocab_ids.find(word) == vocab_ids.end() ){
				id2word[latest_vocab_id] = word;
				vocab_ids[word] = latest_vocab_id++;

			}
			if (vocab_counters.find(word) == vocab_counters.end() ){
				vocab_counters[word] = 0;
			}
			vocab_counters[word]++;

			int cur_vocab_id = vocab_ids[word];

			if (vocab_ids_doc_all.find(cur_vocab_id) == vocab_ids_doc_all.end() ) {
				vocab_ids_doc_all[cur_vocab_id] = 1;
			}else {
				vocab_ids_doc_all[cur_vocab_id]++;
			}

		}
		n_word_type = latest_vocab_id;
		int i=0;
		word_indexes[d] = new int[vocab_ids_doc_all.size()];
		word_counts[d] = new unsigned short[vocab_ids_doc_all.size()];

		for (auto detail : vocab_ids_doc_all) {
			word_indexes[d][i] = detail.first;
			word_counts[d][i] = detail.second;
			n_word_each_doc[d] += word_counts[d][i];
			i++;
		}
		n_word_type_each_doc[d] = vocab_ids_doc_all.size();
		n_all_word += n_word_each_doc[d];
		d++;
	}

	int n_topics = 10;
	int n_show = 20;
	SvbSlda slda(n_topics, 1000);
	slda.fit(word_indexes, word_counts, n_word_each_doc, n_word_type_each_doc, n_all_word, n_document, n_word_type, sample_y, 0);

	double	top_value[n_show];
	int top_index[n_show];
	for(int k=0; k < n_topics; k++) {
		for(int i=0; i < n_show; i++) {
			top_value[i] = 0.;
			top_index[i] = -1;
		}
		for(int w=0; w < n_word_type; w++) {
			for(int i=0; i < n_show; i++) {
				if (slda.phi[k][w] > top_value[i]) {
					for(int j=(n_show - 2); j >= i ; j--) {
						top_value[j + 1] = top_value[j];
						top_index[j + 1] = top_index[j];
					}
					top_value[i] = slda.phi[k][w];
					top_index[i] = w;
					break;
				}
			}
		}
		cout << "TOPIC" << k << endl;
		for(int i=0; i < n_show; i++) {
			cout << "\t" << id2word[top_index[i]] << "(" << top_value[i] << ")" << endl;
		}
		cout << endl;
	}
}
