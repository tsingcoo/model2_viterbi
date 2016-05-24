//
//  main.cpp
//  model2_viterbi
//
//  Created by 王青龙 on 5/23/16.
//  Copyright © 2016 王青龙. All rights reserved.
//

#include <iostream>
#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

void read_index_word(string str,map<string, int>& word2index){//读vcb文件，存储Word和index的映射
    ifstream fin(str);
    string line;
    string word;
    int index;

    while (getline(fin,line)) {
        istringstream linestream(line);
        linestream>>index;
        linestream>>word;
        word2index.insert({word,index});
    }
    cout<<word2index.size()<<endl;
}

void read_corpus(string str, string str2,map<string, int>& word2index){//利用Word和index的映射把语料转成index
    ifstream fin(str);
    ofstream fout(str2);
    string line;
    string word;
    int index;
    bool firstword=true;

    while(getline(fin, line)){
        firstword=true;
        istringstream linestream(line);
        while(linestream>>word){
            if(firstword){
                firstword=false;
            }else{
                fout<<" ";
            }
            if(word2index.find(word)==word2index.end()){
                fout<<-1;//对于这些未登录词可以暂时用-1表示，设置固定的概率
            }else{
                index=word2index[word];
                fout<<index;
            }
        }
        fout<<endl;
    }
}

void read_t(string str, map<int, map<int, double>>& t){
    ifstream fin(str);
    string line;
    int index_ch;
    int index_en;
    double prop;
    while (getline(fin, line)) {
        istringstream linestream(line);
        linestream>>index_ch;
        if(t.find(index_ch)==t.end()){
            t[index_ch]=map<int, double>();
        }
        linestream>>index_en;
        linestream>>prop;
        t[index_ch].insert({index_en, prop});
    }
}

void read_a_g(string str, map<int, map<int, map<int, double>>>& a){//用Giza的格式
    ifstream fin(str);
    string line;
    int len_ch;
    int index_ch;
    int index_en;
    int index_en_my;
    double prop;
    while (getline(fin, line)) {
        istringstream linestream(line);
        linestream>>index_ch;//源语言位置
        linestream>>index_en;//目标语言位置
        index_en_my=index_en-1;
        linestream>>len_ch;//源语言长度
        linestream>>prop;//跳过这个
        linestream>>prop;//对齐概率
        if (a.find(len_ch)==a.end()) {
            a[len_ch]=map<int, map<int, double>>();
        }
        if(a[len_ch].find(index_en_my)==a[len_ch].end()){
            a[len_ch][index_en_my]=map<int, double>();
        }
        a[len_ch][index_en_my].insert({index_ch, prop});
    }
}

void read_index_en(string str, vector<vector<int>> &vv){//存储index表示的目标文件
    ifstream fin(str);
    string line;
    int index;
    int i=0;

    while (getline(fin, line)) {
        istringstream linestream(line);
        vv.push_back(vector<int>());
        while(linestream>>index){
            vv[i].push_back(index);
        }
        ++i;
    }
}

void read_index_ch(string str, vector<vector<int>> &vv){//存储index表示的源文件
    ifstream fin(str);
    string line;
    int index;
    int i=0;

    while (getline(fin, line)) {
        istringstream linestream(line);
        vv.push_back(vector<int>());
        vv[i].push_back(0);//在每一行开头增加一个空位置
        while (linestream>>index) {
            vv[i].push_back(index);
        }
        ++i;
    }
}

void model2_align_g(string str, vector<vector<int>>& vv_ch, vector<vector<int>>& vv_en, map<int, map<int, double>>& prop_t, map<int,map<int,map<int,double>>>& prop_a){
    ofstream fout(str);
    bool firstword=true;
    double tmp_max=0.0;
    int tmp_i=0;
    int lc=(int)vv_ch[0].size();

    for (int l=0; l<vv_ch.size(); ++l) {
        firstword=true;
        lc=(int)vv_ch[l].size()-1;//去掉空之后是源语言真是长度
        vector<int> viterbi_alignment(vv_en[l].size());
        for (int j=0; j<vv_en[l].size(); ++j) {
            tmp_max=0.0;
            tmp_i=0;
            for (int i=0; i<vv_ch[l].size(); ++i) {
                if (prop_t.find(vv_ch[l][i])!=prop_t.end()&&prop_t[vv_ch[l][i]].find(vv_en[l][j])!=prop_t[vv_ch[l][i]].end()&&prop_a.find(lc)!=prop_a.end()&&prop_a[lc].find(j)!=prop_a[lc].end()&&prop_a[lc][j].find(i)!=prop_a[lc][j].end()) {
                    if (tmp_max<prop_t[vv_ch[l][i]][vv_en[l][j]]*prop_a[lc][j][i]) {//这儿还可以多加几个判断
                        tmp_max=prop_t[vv_ch[l][i]][vv_en[l][j]]*prop_a[lc][j][i];
                        tmp_i=i;
                    }
                }else{
                    if (prop_t.find(vv_ch[l][i])!=prop_t.end()&&prop_t[vv_ch[l][i]].find(vv_en[l][j])!=prop_t[vv_ch[l][i]].end()) {
                        if (tmp_max<prop_t[vv_ch[l][i]][vv_en[l][j]]*0.0000001) {
                            tmp_max=prop_t[vv_ch[l][i]][vv_en[l][j]]*0.0000001;
                            tmp_i=i;
                        }
                    }
                    if (prop_a.find(lc)!=prop_a.end()&&prop_a[lc].find(j)!=prop_a[lc].end()&&prop_a[lc][j].find(i)!=prop_a[lc][j].end()) {
                        if (tmp_max<0.0000001*prop_a[lc][j][i]) {
                            tmp_max=0.0000001*prop_a[lc][j][i];
                            tmp_i=i;
                        }
                    }
                    //还有一种两类都却是，可以忽略了
                }

            }
            viterbi_alignment[j]=tmp_i;
        }
        for (int j=0; j!=vv_en[l].size(); ++j) {
            if (viterbi_alignment[j]) {
                if (firstword) {
                    firstword=false;
                }else{
                    fout<<" ";
                }
                fout<<viterbi_alignment[j]-1<<"-"<<j;
            }
        }
        fout<<endl;
    }
    fout.close();
}

int main(int argc, const char * argv[]) {
    // insert code here...
    string dir_name="/Users/wangql/Desktop/model2_viterbi";
    map<string, int> word2index_ch, word2index_en;
    read_index_word(dir_name+"/corpus.ch.vcb", word2index_ch);//把index2word转储成word2index
    read_index_word(dir_name+"/corpus.en.vcb", word2index_en);

    map<int, map<int, double>> t;
    read_t(dir_name + "/s2t64.t2.4", t);

    map<int, map<int, map<int, double>>> a;
    read_a_g(dir_name + "/s2t64.a2.4", a);

    read_corpus(dir_name + "/corpus.ch", dir_name + "/corpus.ch.index", word2index_ch);//把Word转储成index形式
    read_corpus(dir_name + "/corpus.en", dir_name + "/corpus.en.index", word2index_en);

    vector<vector<int>> vv_ch, vv_en;
    read_index_ch(dir_name + "/corpus.ch.index", vv_ch);//读入index形式表示的语料
    read_index_en(dir_name + "/corpus.en.index", vv_en);

    model2_align_g(dir_name + "/model2_g.align", vv_ch, vv_en, t, a);
    return 0;
}
