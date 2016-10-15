//
//  main.cpp
//  PageRank
//
//  Created by Nancy Fan on 16/10/12.
//  Copyright © 2016年 Nancy Fan. All rights reserved.
//

#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <string>
#include <cstring>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>

using namespace std;



//#define PAPER
#define AUTHOR
//#define VENUE
//#define DEBUG



#define max(a, b) ((a>b)? a:b)

static double graph[25000][25000];
static map<string, int> dic;
static map<int, string> redic;
static int nout[25000];
int N = 0;

const double DUMPING_FACTOR = 0.85;
const int MAX_INTERATION = 100;
const double DELTA = 0.00001;
double change = 0;
bool STOP = false;



static struct SCORE{
    double point;
    int id;
    
}score[25000];

int pagerankcmp( const void *a , const void *b )
{
    SCORE A = *(SCORE *)a;
    SCORE B = *(SCORE *)b;
    return A.point < B.point ? 1 : -1;
}


#ifdef DEBUG
void printGraph(){
    for(int i = 1; i <= N; i++){
        for(int j = 1; j <= N; j++){
            cout << graph[i][j]<<" ";
        }
        cout << endl;
    }
    
}

void printScore(){
    for(int i = 1; i <= N; i++){
        cout << score[i].point << endl;
    }
    
}

void printDic(){
    for(int i = 1; i<=N; i++){
        cout<<i<<"-------"<<redic[i]<<endl;
    }
    
    map<string,int>::iterator it;
    for(it = dic.begin(); it!=dic.end(); it++)
        cout<<it->first<<"  "<<it->second<<endl;
    
}
#endif


static int callback(void *data, int argc, char **argv, char **azColName){
    strcpy((char *)data, argv[0]);
    return 0;
}


void read_graph()
{
    string snode1, snode2, str; // node2 is aimed to by node1
 
#ifdef VENUE
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    string sql = "";
    char data[400];
    
    rc = sqlite3_open("../db/paperDB.db", &db);

    if( rc ){
       fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
       exit(-1);
    }
#endif

    while(cin >> snode1 >> str >> snode2){
        int i, j;
#ifdef VENUE
        sql = "SELECT VENUE FROM VENUE WHERE ID == \"" + snode1 +"\"";
        rc = sqlite3_exec(db, sql.c_str(), callback, (void*)data, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            exit(-2);
        }
        snode1 = data;
        
        sql = "SELECT VENUE FROM VENUE WHERE ID == \"" + snode2 +"\"";
        rc = sqlite3_exec(db, sql.c_str(), callback, (void*)data, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            exit(-2);
        }
        snode2 = data;

        //cout << snode1 << "<==" << snode2 << endl;
#endif

        // add into dics
        if(dic.find(snode1)==dic.end()){
            N++;
            dic.insert(make_pair(snode1, N));
            redic.insert(make_pair(N, snode1));
        }
        if(dic.find(snode2)==dic.end()){
            N++;
            dic.insert(make_pair(snode2, N));
            redic.insert(make_pair(N, snode2));
        }
        
        // add into graph
        i = dic[snode1];
        j = dic[snode2];
        if(snode1!=snode2 && graph[i][j]==0){
            graph[i][j]=1;
            nout[i]++;
        }
    }//scanf
    
#ifdef VENUE        
    sqlite3_close(db);
#endif 

    //printDic();
    
    for(int i = 1; i <= N; i++){
        //printf("nout[%d]=%d\n",i,nout[i]);
        if(nout[i]!=0 && nout[i]!=1){
            for(int j = 1; j <= N; j++){
                if(graph[i][j]){
                    graph[i][j] /=(double)N;
                }
            }
        }
    }
    
    // printGraph();
    
    // cout <<endl;
    
    // trans
    for(int i = 1; i <= N; i++){
        for(int j = i + 1; j <= N; j++){
            double tmp = graph[i][j];
            graph[i][j] = graph[j][i];
            graph[j][i] = tmp;
        }
    }
    
    //printGraph();

}

void initcal()
{
    for(int i = 1; i <= N; i++){
        score[i].point = 0;
        score[i].id = i;
        for(int j = 1; j <= N; j++){
            graph[i][j] = DUMPING_FACTOR * graph[i][j] + (1-DUMPING_FACTOR)*1/N;
            score[i].point+=graph[i][j];
        }
    }
}

void calRank()
{
    double * tmp = new double [N];
    change = 0;
    
    for(int i = 1; i <= N; i++){
        tmp[i] = 0;
        for(int j = 1; j <= N; j++){
            tmp[i]+=graph[i][j]*score[j].point;
        }
    }
    
    for(int i = 1; i <= N; i++){
        double changethistime = score[i].point - tmp[1];
        if(changethistime<0)
            changethistime = -changethistime;
        change = max(change, changethistime);
        score[i].point = tmp[i];
    }
    
    if(change <= DELTA) STOP = true;
    
}

int main(int argc, const char * argv[]) {
    memset(graph, 0, sizeof(graph));
    memset(nout, 0, sizeof(nout));
    STOP = false;
    
    
    read_graph();
    initcal();
    //printScore();
    int cnt = 0;
    while(STOP == false){
        calRank();
        //printScore();
        cnt ++;
        if(cnt > MAX_INTERATION) break;
    }
    
    qsort(&score[1],N, sizeof(SCORE), pagerankcmp);

#ifdef VENUE
    for(int i = 1; i <= N; i++){
        cout << redic[score[i].id]<<"\t"<<score[i].point<<endl;
    }
#endif  

#ifdef PAPER 
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    string sql = "";
    char title[500];
    rc = sqlite3_open("../db/paperDB.db", &db);
    if( rc ){
           fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
           exit(-1);
    }

    for(int i = 1; i <= N; i++){
        string paperid = redic[score[i].id];
        sql = "SELECT TITLE FROM TITLE WHERE ID == \"" + paperid +"\"";
        rc = sqlite3_exec(db, sql.c_str(), callback, (void*)title, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            exit(-2);
        }

        cout << title <<"\t"<<score[i].point<<endl;
    }
    sqlite3_close(db);
#endif

#ifdef AUTHOR
    sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    string sql = "";
    char author[500];
    rc = sqlite3_open("../db/paperDB.db", &db);
    if( rc ){
       fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
       exit(-1);
    }

    for(int i = 1; i <= N; i++){
        string authorid = redic[score[i].id];
        sql = "SELECT NAME FROM AUTHOR WHERE AUTHOR_ID == \"" + authorid +"\"";
        rc = sqlite3_exec(db, sql.c_str(), callback, (void*)author, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            exit(-2);
        }

        cout << author <<"\t"<<score[i].point<<endl;
    }
    sqlite3_close(db);
#endif
    return 0;
}









