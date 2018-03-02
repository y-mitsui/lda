
class LDA {
private:
	int getSample(const char *path, int ***word_indexes_r, unsigned short ***word_counts_r, int **n_word_type_each_doc_r, int **n_word_each_doc_r, unsigned long long *n_all_word_r, int *n_document_r, int *n_word_type_r);
	unsigned long xor128();
public:
	double **phi;
	double **theta;
	void fitFile(const char *path);
};
