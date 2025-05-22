#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <bitset>
using namespace std;

bool checkCRC(string codeword, string generator) {
    int codeword_len = codeword.length();
    int generator_len = generator.length();

    for (int i = 0; i <= codeword_len - generator_len;) {
        for (int j = 0; j < generator_len; j++) {
            codeword[i + j] = (codeword[i + j] == generator[j] ? '0' : '1');
        }
        while (i < codeword_len && codeword[i] != '1') i++;
    }

    return codeword.substr(codeword_len - generator_len + 1).find('1') == string::npos;
}

void decode(char* input_file, char* output_file, char* result_file, string generator, int dataword_size) {
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

    ofstream rfs(result_file);
    if (!rfs) {
        cerr << "result file open error.\n";
        return;
    }

    char byte;
    ifs.read(&byte, 1); 
    int padding_bits = bitset<8>(byte).to_ulong();

    vector<bitset<8> > bytes;
    while (ifs.read(&byte, 1)) {
        bytes.push_back(bitset<8>(byte));
    }

    string bitstring;
    for (size_t i = 0; i < bytes.size(); i++) {
        bitstring += bytes[i].to_string();
    }

    bitstring = bitstring.substr(padding_bits);

    int total_codewords = 0;
    int error_codewords = 0;

    string ans;
    for (size_t i = 0; i < bitstring.length(); i += dataword_size + generator.length() - 1) {
        string codeword = bitstring.substr(i, dataword_size + generator.length() - 1);
        bool correct = checkCRC(codeword, generator);
        if (!correct) error_codewords++;
        ans += codeword.substr(0, dataword_size);
        total_codewords++;
    }

    for (int i = 0; i < ans.length(); i += 8) {
        ofs << (char)bitset<8>(ans.substr(i, 8)).to_ulong();
    }

    rfs << total_codewords << ' ' << error_codewords;

    ifs.close();
    ofs.close();
    rfs.close();
}

int main(int argc, char* argv[]) {
    if (argc != 6) {
        cerr << "usage: ./crc_decoder input_file output_file result_file generator dataword_size\n";
        return 1;
    }

    int dataword_size = atoi(argv[5]);
    if (dataword_size != 4 && dataword_size != 8) {
        cerr << "dataword size must be 4 or 8.\n";
        return 1;
    }

    decode(argv[1], argv[2], argv[3], argv[4], dataword_size);
    return 0;
}