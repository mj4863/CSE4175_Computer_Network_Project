#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>
using namespace std;

string calculateCRC (string dataword, string generator) {
    int dataword_len = dataword.length();
    int generator_len = generator.length();

    string dividend = dataword + string(generator_len - 1, '0');
    int dividend_len = dividend.length();

    for (int i = 0; i <= dividend_len - generator_len;) {
        for (int j = 0; j < generator_len; j++) {
            dividend[i + j] = (dividend[i + j] == generator[j] ? '0' : '1');
        }
        while (i < dividend_len && dividend[i] != '1') i++;
    }

    return dividend.substr(dividend_len - generator_len + 1);
}

void encode(char* input_file, char* output_file, string generator, int dataword_size) {
    ifstream ifs(input_file, ios::binary);
    if (!ifs) {
        cerr << "input file open error.\n";
        return;
    }

    ofstream ofs(output_file, ios::binary);
    if (!ofs) {
        cerr << "output file open error.\n";
        return;
    }

    char byte;
    vector<bitset<8> > bytes;
    while (ifs.read(&byte, 1)) {
        bytes.push_back(bitset<8>(byte));
    }

    string bitstring;
    for (size_t i = 0; i < bytes.size(); i++) {
        bitstring += bytes[i].to_string();
    }

    int word_cnt = bitstring.length() / dataword_size;
    int codewords_length = bitstring.length() + word_cnt * (generator.length() - 1);
    int padding_bits = (8 - (codewords_length % 8)) % 8;
    ofs << (char)bitset<8>(padding_bits).to_ulong();

    string ans;
    for (size_t i = 0; i < bitstring.length(); i += dataword_size) {
        string dataword = bitstring.substr(i, dataword_size);
        string crc = calculateCRC(dataword, generator);
        string codeword = dataword + crc;
        ans += codeword;
    }
    ans = string(padding_bits, '0') + ans;
    
    for (int i = 0; i < ans.length(); i += 8) {
        ofs << (char)bitset<8>(ans.substr(i, 8)).to_ulong();
    }

    ifs.close();
    ofs.close();
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        cerr << "usage: ./crc_encoder input_file output_file generator dataword_size\n";
        return 1;
    }

    int dataword_size = atoi(argv[4]);
    if (dataword_size != 4 && dataword_size != 8) {
        cerr << "dataword size must be 4 or 8.\n";
        return 1;
    }

    encode(argv[1], argv[2], argv[3], dataword_size);
    return 0;
}