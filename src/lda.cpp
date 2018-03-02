
unsigned long LDA::xor128(){
    static unsigned long x=123456789,y=362436069,z=521288629,w=88675123;
    unsigned long t;
    t=(x^(x<<11));x=y;y=z;z=w; return( w=(w^(w>>19))^(t^(t>>8)) );
}

void LDA::fitFile(const char *path) {
	int **word_indexes;
	unsigned short **word_counts;
	int *n_word_type_each_doc, *n_word_each_doc;
	int n_document, n_word_type;
	unsigned long long n_all_word;

	getSample(path, &word_indexes, &word_counts, &n_word_type_each_doc, &n_word_each_doc, &n_all_word, &n_document, &n_word_type);
	fit(word_indexes, word_counts, n_word_each_doc, n_word_type_each_doc, n_all_word, n_document, n_word_type);
	for(int i=0; i < n_document; i++) {
		delete[] word_indexes[i];
		delete[] word_counts[i];
	}
	delete[] word_indexes;
	delete[] word_counts;
	delete[] n_word_type_each_doc;
	delete[] n_word_each_doc;
}

int LDA::getSample(const char *path, int ***word_indexes_r, unsigned short ***word_counts_r, int **n_word_type_each_doc_r, int **n_word_each_doc_r, unsigned long long *n_all_word_r, int *n_document_r, int *n_word_type_r){
	int n_document;
	FILE *fp_word_indexes = fopen(path, "rb");
	if(!fp_word_indexes) return -1;
	fread(&n_document, sizeof(int), 1, fp_word_indexes);
	int **word_indexes = new int*[n_document];
	unsigned short **word_counts = new unsigned short*[n_document];
	int *n_word_type_each_doc = new int[n_document];
	int *n_word_each_doc = new int[n_document];
	unsigned long long n_all_word = 0;
	int n_word_type=0;
	int i, j;
	for (i=0; i < n_document; i++) {
		fread(&n_word_type_each_doc[i], sizeof(int), 1, fp_word_indexes);
		word_indexes[i] = new int[n_word_type_each_doc[i]];
		word_counts[i] = new unsigned short[n_word_type_each_doc[i]];
		for (j=0; j < n_word_type_each_doc[i]; j++) {
			fread(&word_indexes[i][j], sizeof(int), 1, fp_word_indexes);
			fread(&word_counts[i][j], sizeof(unsigned short), 1, fp_word_indexes);
			n_word_each_doc[i] += word_counts[i][j];
			n_all_word += word_counts[i][j];
			if (n_word_type < word_indexes[i][j])
				n_word_type = word_indexes[i][j];
		}
	}
	fclose(fp_word_indexes);

	n_word_type++;
	*word_indexes_r = word_indexes;
	*word_counts_r = word_counts;
	*n_word_type_each_doc_r = n_word_type_each_doc;
	*n_word_each_doc_r = n_word_each_doc;
	*n_all_word_r = n_all_word;
	*n_document_r = n_document;
	*n_word_type_r = n_word_type;
	return 0;
}
