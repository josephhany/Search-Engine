#include<bits/stdc++.h>
using namespace std;

struct Edge {
	string src, dest;
};

class Graph
{
	map<string,int> links_nodes;
	vector<vector<int>> In_Degree;
	vector<vector<int>> adjList;
	vector<float> old_PageRank;
	vector<float> PageRank;
	vector<float> PageScore;
	vector<pair <string,float>> search_results;
	vector<vector<string>> parsed_query;
	map<string, vector<string>> links_keywords;
	vector<string> final_search_results;
public:
    vector<int> impressions;
    vector<string> all_links;
    vector<int> clicks;
	Graph(vector<Edge> const &edges,vector<vector<string>> keywords,vector<vector<string>> impressions_init,vector<vector<string>> clicks_init);
	void pagerank();
	void normalize_pagerank(float max_PR,float min_PR);
	void parse_query_or(string or_query,string dlim);
	void parse_query(string query,string dlim,string dlim2);
	vector<string> quotes_spaces(vector<string> &vect,string str);
	void search_query();
	void Update_Pages_Score();
	void view_last_search_results();
	void view_page(int i);
	static bool sortbysec(const pair<string,float> &a, const pair<string,float> &b);
};

void init_var(vector<vector<string>> &var,string filename){
    ifstream input_file;
    input_file.open (filename);
    string str;
        while(getline(input_file, str)){
                vector <string> tokens;
                stringstream check(str);
                string intermediate;
                while(getline(check, intermediate, ','))
                {
                    tokens.push_back(intermediate);
                }
                var.push_back(tokens);
        }
    input_file.close();
}

void fill_vect(vector<Edge> &vect,string filename){
    ifstream input_file;
    input_file.open (filename);
    string str1;
        while(getline(input_file, str1)){
                stringstream check1(str1);
                string intermediate;
                Edge edge;
                getline(check1, intermediate, ',');
                edge.src=intermediate;
                getline(check1, intermediate, ',');
                edge.dest=intermediate;
                vect.push_back(edge);
        }
    input_file.close();
}

void fill_file(Graph graph,vector<int> vect,string filename){
    ofstream myfile;
    myfile.open (filename);
    for(int i=0;i<vect.size();i++){
            myfile <<graph.all_links[i]<<","<<vect[i]<<"\n";
    }
    myfile.close();
}

int main(){
        ifstream input_file;
        vector<vector<string>> keywords,impressions_init,clicks_init;
        init_var(keywords,"keywords.csv");
        init_var(impressions_init,"impressions.csv");
        init_var(clicks_init,"clicks.csv");
        vector<Edge> edges;
        fill_vect(edges,"edges.csv");

		Graph graph(edges,keywords,impressions_init,clicks_init);
		graph.pagerank();
		cout<<"welcome!\nWhat would you like to do?\n1.\tNew search\n2.\tExit\nType in your choice:\n";
		int choice;
		cin>>choice;
		if(choice==1){
            choice=2;
		}
		else{
            choice=1;
		}
		while(choice==2){
			cout<<"Enter your search query:\n";
			string query;
			std::cin.ignore() ;
			std::getline(cin,query);
			graph.parse_query(query,"AND ","OR ");
			graph.search_query();
			do{
				graph.view_last_search_results();
				cout<<"\nwould you like to\n1.\tchoose a webpage to open\n2.\tNew Search\n3.\tExit\nType in Your choice:\n";
				cin>>choice;
				if(choice==1){
					cout<<"Which webpage you want to open?\nType in your choice:\n";
					int page_num;
					cin>>page_num;
					graph.view_page(page_num);
					cout<<"Would you like to\n1.\tBack to search results\n2.\tNew Search\n3.\tExit\nType in your choice:\n";
					cin>>choice;
				}
			}while(choice==1);
		}
		cout<<endl;


		fill_file(graph,graph.impressions,"impressions.csv");
		fill_file(graph,graph.clicks,"clicks.csv");

	return 0;
}

Graph::Graph(vector<Edge> const &edges,vector<vector<string>> keywords,vector<vector<string>> impressions_init,vector<vector<string>> clicks_init){
	int node_num=0;

	//initialize edges
	for (auto &edge: edges) {
		//search for edge.src and edge.dest in the map if not found add
		//them to the map as a key with a value corresponding to the node number
		auto it = links_nodes.find(edge.src);
    	if(it == links_nodes.end()){
    		links_nodes[edge.src]=node_num;
    		all_links.push_back(edge.src);
    		node_num++;
    	}

		auto it2 = links_nodes.find(edge.dest);
    	if(it2 == links_nodes.end()){
    		links_nodes[edge.dest]=node_num;
    		all_links.push_back(edge.dest);
    		node_num++;
    	}


		if(links_nodes[edge.src]>=adjList.size() || links_nodes[edge.dest]>=adjList.size()){
			adjList.resize(max(links_nodes[edge.src],links_nodes[edge.dest])+1);
			In_Degree.resize(max(links_nodes[edge.src],links_nodes[edge.dest])+1);
		}
		(adjList[links_nodes[edge.src]]).push_back(links_nodes[edge.dest]);
		(In_Degree[links_nodes[edge.dest]]).push_back(links_nodes[edge.src]);
	}

	//solving Dnagiling nodes issue
	for(int i=0;i<adjList.size();i++){
		if(adjList[i].size()==0){
			for(int j=0;j<adjList.size();j++){
				if(j!=i){
					adjList[i].push_back(j);
					In_Degree[j].push_back(i);
				}
			}
		}
	}

	//intialize keywords
	for(int i=0;i<keywords.size();i++){
		for(int j=1;j<keywords[i].size();j++){
			links_keywords[keywords[i][j]].push_back(keywords[i][0]);
		}
	}

	//intialize impressions
	impressions.resize(all_links.size(),1);
	for(int i=0;i<impressions_init.size();i++){
            impressions[links_nodes[impressions_init[i][0]]]=stoi(impressions_init[i][1]);
	}

	//intialize clicks
	clicks.resize(all_links.size(),0);
	for(int i=0;i<clicks_init.size();i++){
            clicks[links_nodes[clicks_init[i][0]]]=stoi(clicks_init[i][1]);
	}

}

void Graph::pagerank(){

	//initializing the pagerank with 1/n
	float size=adjList.size();
	PageRank.resize(adjList.size(),1/size);

	//main page rank loop
	int max_iterations=10000;
	double sigma_tolerance=1e-12;
	int n_iteration=0;
	float diff=1;
	bool diff_flag=true;
	float min_PR=FLT_MAX;
	float max_PR=FLT_MIN;
	while(diff_flag && n_iteration<max_iterations){
        old_PageRank=PageRank;
		diff_flag=false;
		for(int i=0; i<PageRank.size();i++){
			PageRank[i]=0;
			for(int j=0;j<In_Degree[i].size(); j++){
				float Out_Degree=adjList[In_Degree[i][j]].size();
				PageRank[i]+=(old_PageRank[In_Degree[i][j]]/Out_Degree);
			}
			if(PageRank[i]>max_PR)max_PR=PageRank[i];
			if(PageRank[i]<min_PR)min_PR=PageRank[i];
			diff=abs(PageRank[i]-old_PageRank[i]);
			if(diff>sigma_tolerance)diff_flag=true;
		}
		n_iteration++;
	}

    //normalize the page rank
	normalize_pagerank(max_PR,min_PR);

	// init PageScore
	PageScore.resize(PageRank.size());
	for(int i=0;i<PageRank.size();i++){
		PageScore[i]=PageRank[i];
	}
}

void Graph::normalize_pagerank(float max_PR, float min_PR){
    for(int i=0;i<old_PageRank.size();i++){
		PageRank[i]=(PageRank[i]-min_PR)/(max_PR-min_PR);
    }
}

void Graph::parse_query_or(string or_query,string dlim){
		int current_or, previous_or = 0;
		current_or = or_query.find(dlim);
		vector<string> sub_parsed_query;
		while (current_or != std::string::npos) {
			quotes_spaces(sub_parsed_query,or_query.substr(previous_or, current_or-1 - previous_or));
			previous_or = current_or + dlim.size();
        	current_or = or_query.find(dlim, previous_or);
		}
		quotes_spaces(sub_parsed_query,or_query.substr(previous_or, current_or-1 - previous_or));
		parsed_query.push_back(sub_parsed_query);
}

void Graph::parse_query(string query,string dlim,string dlim2){
    parsed_query.clear();
	int current_and, previous_and = 0;
	current_and = query.find(dlim);
	while (current_and != std::string::npos) {
		string or_query=query.substr(previous_and, current_and-1 - previous_and);
		parse_query_or(or_query,dlim2);
        previous_and = current_and + dlim.size();
        current_and = query.find(dlim, previous_and);
    }
    string new_query=query.substr(previous_and, current_and-1 - previous_and);
    parse_query_or(new_query,dlim2);
}

vector<string> Graph::quotes_spaces(vector<string> &vect,string str){
      	int i=0;
        while(i<str.size()){
	        if (str[i] == '"')
	        {
	        	i++;
	        	int start=i;
	        	int end;
	        	string extra;
	        	while(str[i]!='"' && i!=str.size()){
	        		i++;
	        	}
	        	end=i;
	            vect.push_back(str.substr(start,end-start));
	            i++;
	        }
	        else if((str[i]==' ' && str[i+1]!='"' && str[i+1]!=' ')|| (i==0 && str[i]!=' '&& str[i]!='"'))
	        {
	        	if(i!=0)i++;
	        	int start=i;
	        	int end;
	        	string extra;
	        	while(str[i]!=' ' && i!=str.size()){
	        		i++;
	        	}
	        	end=i;
	            vect.push_back(str.substr(start,end-start));
	        }
	        else{
	        	i++;
	        }
        }
        return vect;
      }

bool Graph::sortbysec(const pair<string,float> &a, const pair<string,float> &b)
{
    return (a.second > b.second);
}

void Graph::search_query(){
	//match the parsed query with the links keywords
	vector<string> current_search_results;
	final_search_results.clear();
	search_results.clear();

	for(int i=0; i<parsed_query.size(); i++){
		for(int j=0; j<parsed_query[i].size(); j++){
			if(links_keywords[parsed_query[i][j]].size()>0){
				if(j!=0){
				sort(links_keywords[parsed_query[i][j]].begin(), links_keywords[parsed_query[i][j]].end());
				vector<string> v(current_search_results.size() + links_keywords[parsed_query[i][j]].size());
    			auto it=std::set_union(links_keywords[parsed_query[i][j]].begin(),links_keywords[parsed_query[i][j]].end(),
    									current_search_results.begin(),current_search_results.end(),
    									v.begin());
    			v.resize(it-v.begin());
    			current_search_results.clear();
    			current_search_results=v;
				}else{current_search_results=links_keywords[parsed_query[i][j]];}
			}
		}
		if(i!=0){
			vector<string> v(current_search_results.size() + final_search_results.size());
			auto it = set_intersection(current_search_results.begin(), current_search_results.end(), final_search_results.begin(), final_search_results.end(), v.begin());
			v.resize(it-v.begin());
			final_search_results.clear();
			final_search_results=v;
		}else{final_search_results=current_search_results;

		}
	}

	//copy final_search_results in search_results
	for(int i=0;i<final_search_results.size();i++){
		search_results.push_back(make_pair(final_search_results[i],PageScore[links_nodes[final_search_results[i]]]));

	}

	// sorted links according to their score
	sort(search_results.begin(),search_results.end(),sortbysec);

	//update impressions
	for(int i=0;i<final_search_results.size();i++){
		int n=links_nodes[final_search_results[i]];
		impressions[n]++;
	}
	//update PageScore
	Update_Pages_Score();


}

void Graph::view_last_search_results(){
	cout<<"search results:\n";
	for(int i=0;i<search_results.size();i++){
		cout<<i+1<<".\t"<<search_results[i].first<<endl;
	}
}

void Graph::Update_Pages_Score(){
	for(int i=0;i<final_search_results.size();i++){
		float impression=impressions[links_nodes[final_search_results[i]]];
		int n=links_nodes[final_search_results[i]];
		float PRnorm=PageRank[n];
		float CTR=clicks[n]/impression;
		PageScore[n]=0.4*PRnorm+((1-((0.1*impression)/(0.1*impression+1)))*PRnorm+((0.1*impression*CTR)/(0.1*impression+1)))*0.6;
	}
}

void Graph::view_page(int i){
		cout<<"You're now viewing "<<search_results[i-1].first<<".\n";
		//update the clicks
		clicks[links_nodes[search_results[i-1].first]]++;
		Update_Pages_Score();
}
