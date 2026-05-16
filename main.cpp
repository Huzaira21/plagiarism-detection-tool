#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <cmath>
#include <map>

using namespace std;

// List of common words to ignore during word-level comparison
const unordered_set<string> commonWords = {"the", "is", "and", "a", "an", "of", "to", "in", "it", "on", "for", "with", "at", "by", "from", "are"};

// Synonym dictionary for dynamic synonym detection
unordered_map<string, string> synonyms;

// Function to load synonyms from a file
void loadSynonyms(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error opening synonyms file: " << filename << endl;
        return;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string word1, word2;
        if (ss >> word1 >> word2) {
            synonyms[word1] = word2;
            synonyms[word2] = word1; // Make the mapping bidirectional
        }
    }
    file.close();
}

// Function to create empty files if they do not exist
void createFiles() {
    ofstream file1("file1.txt", ios::app);
    ofstream file2("file2.txt", ios::app);
    ofstream synonymsFile("synonyms.txt", ios::app);

    if (file1.is_open()) file1.close();
    if (file2.is_open()) file2.close();
    if (synonymsFile.is_open()) synonymsFile.close();
}

// Function to extract sentences from a file
vector<string> extractSentences(const string& filename) {
    ifstream file(filename);
    vector<string> sentences;
    string line, sentence;

    if (file.is_open()) {
        while (getline(file, line)) {
            stringstream ss(line);
            while (getline(ss, sentence, '.')) {
                for (char& c : sentence) c = tolower(c);
                sentence.erase(remove_if(sentence.begin(), sentence.end(), ::isspace), sentence.end());
                if (!sentence.empty()) {
                    sentences.push_back(sentence);
                }
            }
        }
        file.close();
    } else {
        cerr << "Error opening file: " << filename << endl;
    }

    return sentences;
}

// Function to extract words from a file, excluding common words
set<string> extractWords(const string& filename) {
    ifstream file(filename);
    set<string> words;
    string line, word;

    if (file.is_open()) {
        while (getline(file, line)) {
            stringstream ss(line);
            while (ss >> word) {
                for (char& c : word) c = tolower(c);
                word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
                if (!word.empty() && commonWords.find(word) == commonWords.end()) {
                    words.insert(word);
                }
            }
        }
        file.close();
    } else {
        cerr << "Error opening file: " << filename << endl;
    }

    return words;
}

// Function to calculate sentence-level plagiarism
double calculateSentencePlagiarism(const vector<string>& sentences1, const vector<string>& sentences2) {
    int commonSentences = 0;

    for (const auto& sentence : sentences1) {
        if (find(sentences2.begin(), sentences2.end(), sentence) != sentences2.end()) {
            commonSentences++;
        }
    }

    int totalSentences = sentences1.size() + sentences2.size() - commonSentences;
    return (totalSentences > 0) ? (commonSentences * 100.0) / totalSentences : 0.0;
}

// Function to calculate word-level plagiarism
double calculateWordPlagiarism(const set<string>& words1, const set<string>& words2) {
    int commonWords = 0;

    for (const auto& word : words1) {
        if (words2.find(word) != words2.end()) {
            commonWords++;
        }
    }

    int totalWords = words1.size();
    return (totalWords > 0) ? (commonWords * 100.0) / totalWords : 0.0;
}

// Function to perform synonym detection
double calculateSynonymPlagiarism(const set<string>& words1, const set<string>& words2) {
    int matchingWords = 0;

    for (const auto& word1 : words1) {
        if (words2.find(word1) != words2.end() ||
            (synonyms.find(word1) != synonyms.end() && words2.find(synonyms.at(word1)) != words2.end())) {
            matchingWords++;
        }
    }

    int totalWords = words1.size();
    return (totalWords > 0) ? (matchingWords * 100.0) / totalWords : 0.0;
}

// Function to calculate Levenshtein distance for approximate matching
int levenshteinDistance(const string& s1, const string& s2) {
    int len1 = s1.size(), len2 = s2.size();
    vector<vector<int>> dp(len1 + 1, vector<int>(len2 + 1, 0));

    for (int i = 0; i <= len1; ++i) dp[i][0] = i;
    for (int j = 0; j <= len2; ++j) dp[0][j] = j;

    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            if (s1[i - 1] == s2[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1];
            } else {
                dp[i][j] = 1 + min({dp[i - 1][j], dp[i][j - 1], dp[i - 1][j - 1]});
            }
        }
    }

    return dp[len1][len2];
}

// Function to calculate approximate sentence matching percentage
double calculateApproximateSentenceMatching(const vector<string>& sentences1, const vector<string>& sentences2) {
    int similarSentences = 0;

    for (const auto& sentence1 : sentences1) {
        for (const auto& sentence2 : sentences2) {
            if (levenshteinDistance(sentence1, sentence2) <= 3) { // Allow small differences
                similarSentences++;
                break;
            }
        }
    }

    int totalSentences = sentences1.size();
    return (totalSentences > 0) ? (similarSentences * 100.0) / totalSentences : 0.0;
}

// Function to calculate Cosine Similarity between two sets of words
double calculateCosineSimilarity(const set<string>& words1, const set<string>& words2) {
    map<string, int> wordFrequency1, wordFrequency2;
    int commonWordsCount = 0;

    // Fill word frequencies for file1
    for (const auto& word : words1) {
        wordFrequency1[word]++;
    }

    // Fill word frequencies for file2
    for (const auto& word : words2) {
        wordFrequency2[word]++;
    }

    // Calculate the dot product of the two word frequency vectors
    double dotProduct = 0;
    for (const auto& entry : wordFrequency1) {
        if (wordFrequency2.find(entry.first) != wordFrequency2.end()) {
            dotProduct += entry.second * wordFrequency2.at(entry.first);
        }
    }

    // Calculate the magnitude (length) of both word frequency vectors
    double magnitude1 = 0, magnitude2 = 0;
    for (const auto& entry : wordFrequency1) {
        magnitude1 += entry.second * entry.second;
    }
    for (const auto& entry : wordFrequency2) {
        magnitude2 += entry.second * entry.second;
    }

    magnitude1 = sqrt(magnitude1);
    magnitude2 = sqrt(magnitude2);

    // Cosine Similarity formula
    return (magnitude1 > 0 && magnitude2 > 0) ? (dotProduct / (magnitude1 * magnitude2)) : 0.0;
}

// Function to preprocess text by removing punctuation and converting to lowercase
string preprocess(const string& text) {
    string result;
    for (char c : text) {
        if (isalpha(c) || isspace(c)) {
            result += tolower(c);
        }
    }
    return result;
}

// Function to generate n-grams from a string
set<string> generateNGrams(const string& text, int n) {
    vector<string> words;
    string word;
    stringstream ss(text);

    // Split text into words
    while (ss >> word) {
        // Remove punctuation and convert to lowercase
        word.erase(remove_if(word.begin(), word.end(), ::ispunct), word.end());
        transform(word.begin(), word.end(), word.begin(), ::tolower);
        words.push_back(word);
    }

    // Generate n-grams
    set<string> ngrams;
    for (size_t i = 0; i + n - 1 < words.size(); ++i) {
        string ngram;
        for (int j = 0; j < n; ++j) {
            if (j > 0) ngram += " ";
            ngram += words[i + j];
        }
        ngrams.insert(ngram);
    }
    return ngrams;
}

// Function to calculate n-gram plagiarism
double calculateNGramPlagiarism(const string& file1Content, const string& file2Content, int n) {
    set<string> ngrams1 = generateNGrams(file1Content, n);
    set<string> ngrams2 = generateNGrams(file2Content, n);

    // Count common n-grams
    int commonCount = 0;
    for (const auto& ngram : ngrams1) {
        if (ngrams2.find(ngram) != ngrams2.end()) {
            ++commonCount;
        }
    }

    // Calculate plagiarism percentage
    return (!ngrams1.empty()) ? (commonCount * 100.0 / ngrams1.size()) : 0.0;
}

int main() {
    createFiles();
    loadSynonyms("synonyms.txt");
    cout << "Please write content in file1.txt and file2.txt, then run the program again.\n";

    string file1Content, file2Content;

    // Read content from files
    ifstream file1("file1.txt"), file2("file2.txt");
    if (file1.is_open() && file2.is_open()) {
        string line;

        while (getline(file1, line)) {
            file1Content += line + " ";
        }

        while (getline(file2, line)) {
            file2Content += line + " ";
        }

        file1.close();
        file2.close();
    } else {
        cerr << "Error opening files. Ensure both files exist.\n";
        return 1;
    }

    int choice;
    do {
        cout << "\n--- Menu ---\n";
        cout << "1. Simple Plagiarism Check (Sentence & Word Level)\n";
        cout << "2. Synonym Detection\n";
        cout << "3. Approximate Sentence Matching\n";
        cout << "4. Cosine Similarity (Document-Level Plagiarism)\n";
        cout << "5. N-Gram Plagiarism Detection\n";
        cout << "6. Exit\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: {
                vector<string> sentences1 = extractSentences("file1.txt");
                vector<string> sentences2 = extractSentences("file2.txt");
                set<string> words1 = extractWords("file1.txt");
                set<string> words2 = extractWords("file2.txt");

                double sentencePlagiarism = calculateSentencePlagiarism(sentences1, sentences2);
                double wordPlagiarism = calculateWordPlagiarism(words1, words2);

                cout << "Sentence-Level Plagiarism: " << sentencePlagiarism << "%\n";
                cout << "Word-Level Plagiarism: " << wordPlagiarism << "%\n";
                break;
            }
            case 2: {
                set<string> words1 = extractWords("file1.txt");
                set<string> words2 = extractWords("file2.txt");

                double synonymPlagiarism = calculateSynonymPlagiarism(words1, words2);
                cout << "Synonym-Based Plagiarism: " << synonymPlagiarism << "%\n";
                break;
            }
            case 3: {
                vector<string> sentences1 = extractSentences("file1.txt");
                vector<string> sentences2 = extractSentences("file2.txt");

                double approximateMatching = calculateApproximateSentenceMatching(sentences1, sentences2);
                cout << "Approximate Sentence Matching: " << approximateMatching << "%\n";
                break;
            }
            case 4: {
                set<string> words1 = extractWords("file1.txt");
                set<string> words2 = extractWords("file2.txt");

                double cosineSimilarity = calculateCosineSimilarity(words1, words2);
                cout << "Cosine Similarity: " << cosineSimilarity * 100.0 << "%\n";
                break;
            }
            case 5: {
                int n;
                cout << "Enter the value of n for n-gram plagiarism detection: ";
                cin >> n;

                double nGramPlagiarism = calculateNGramPlagiarism(file1Content, file2Content, n);
                cout << "N-Gram Plagiarism Detection (n=" << n << "): " << nGramPlagiarism << "%\n";
                break;
            }
            case 6:
                cout << "Exiting program.\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
                break;
        }
    } while (choice != 6);

    return 0;
}
