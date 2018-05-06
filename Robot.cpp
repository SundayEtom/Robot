#include <iostream>
#include <regex>
#include <vector>
#include <string>
#include <ctime>
#include <cctype>
#include <fstream>

using namespace std;


enum {eof=-1, ident, num, str, mathop, condop, blank, o_paren, c_paren, equals, assign};

class Scanner{
	private:
		string srcstr;
		int start, forward;
		char current;
	
	public:
		Scanner(string text){
			start = 0;
			forward = 0;
			srcstr = text;
			for(; isspace(srcstr.at(start)); start++);
			forward = start;
			nextChar();
		}
		
		int scan(string& result){
			string tok = "";
			int what = eof;
			
			if(isalpha(current)){
				while(isalnum(current)){
					tok += current;
					nextChar();
				}
				what = ident;
			}
			else if(isdigit(current)){
				while(isdigit(current)){
					tok += current;
					nextChar();
				}
				what = num;
			}
			else if(isspace(current)){
				while(isspace(current)){
					nextChar();
				}
				tok = "";
				what = blank;
			}
			else if(current == '('){
				tok += current;
				what = o_paren;
				nextChar();
			}
			else if(current == ')'){
				tok += current;
				what = c_paren;
				nextChar();
			}
			else if(current == '='){
				char prev = current;
				nextChar();
				if(current == '='){
					tok += "==";
					what = equals;
					nextChar();
				}
				else{
					tok += prev;
					what = assign;
				}
			}
			else if(current == '"'){
				nextChar();
				while(current != '"'){
					char prev = current;
					nextChar();
					if(prev == '\\'){
						if(current == '"')
							tok += current;
						else{
							tok += prev;
							tok += current; 
						}
					}
					else{
						tok += prev;
						tok += current; 
					}
					nextChar();
				}
				nextChar();
				what = str;
			}
			else{
				switch(current){
					case '+':
					case '*':
					case '-':
					case '/':
					case '%': tok = current; what = mathop; break;
				}
				nextChar();
			}
			
			result = tok;
			return what;
		}
		
		
		char nextChar(void){
			if(forward >= srcstr.size())
				current = '\0';
			else
				current = srcstr.at(forward++);
			
			return current;
		}
		
};



class Parser{
		
	public:
		Parser(string s){
			Scanner sc(s);
			string tok;
			int type = sc.scan(tok);
			while(type != eof){
				string typestr;
				switch(type){
					case num: 		typestr = "number    "; 	break;
					case str: 		typestr = "string    "; 	break;
					case mathop: 	typestr = "mathop    "; 	break;
					case blank: 	typestr = "blank     "; 	break;
					case condop: 	typestr = "condop    "; 	break;
					case ident: 	typestr = "identifier"; 	break;
					case o_paren: 	typestr = "o_paren   "; 	break;
					case c_paren: 	typestr = "c_paren   "; 	break;
					case equals:	typestr = "equals    ";		break;
					case assign:	typestr = "assignment";		break; 
				}
				cout << "Type: " << typestr << ", lexeme: '" << tok << "'" << endl;
				type = sc.scan(tok);
			}
		}
};



class Category{
	private:
		string name;
		vector<string> queries;
		vector<string> replies;
	
	public:
		Category(string content, string name){
			this->name = name;
			regex pat{"#[[:s:]]+([[:alnum:]]+)[[:s:]]*=[[:s:]]*(.*?)[[:s:]]+#"};
			
			for(sregex_iterator p(content.begin(), content.end(), pat); p!=sregex_iterator{}; p++){
				if((*p)[1] == "query")
					queries.push_back((*p)[2]);
				else if((*p)[1] == "reply")
					replies.push_back((*p)[2]);
			}
		}
		
		string respond(string query){
			string res = "";
			
			for(string q : queries){
				if(q == query){
					srand((int)time(0));
					int sz{replies.size()};
					int index{rand() % sz};
					res = replies.at(index);
					break;
				}
			}
			if(!res.empty()){
				regex pat{"[{][[:s:]]*(.*?)[[:s:]]*[}]"};
				ofstream os("interpreter.py");
				if(os.is_open()){
					for(sregex_iterator p(res.begin(), res.end(), pat); p != sregex_iterator{}; p++){
						cout << "Code " << (*p)[1] << " skipped..." << endl;
						cout.flush();
					}
				}
				
				os.close();
				res = regex_replace(res, pat, "");
			}
			
			return res;
		}
		
		vector<string> getQueries(void){ return queries; }
		vector<string> getReplies(void){ return replies; }
};



class Robot{
	private:
		string source;
		string name; 
		vector<Category> cats;
		
	public:
		Robot(string srcfile){
			ifstream is(srcfile);
			if(is.is_open()){
				cout << "Source opened." << endl;
				int linecount = 0;
				while(is){
					string line;
					getline(is, line);
					smatch matches;
					if(linecount <= 0 && regex_search(line, matches,  regex("name[[:s:]]*=[[:s:]]*([[:alnum:]]+)"))){
						name = matches[1].str();
						continue;
					}
					source += line;
					linecount++;
				}
				cout << "Robot's name is '" << name << "'" << endl;
				is.close();
			}
			else{ source = srcfile; }
			
			parse();
		}
		
		
		void parse(void){
			if(name.empty()){
				regex namepat{"name[[:s:]]*=[[:s:]]*([[:alnum:]]+)[[:s:]]+"};
				smatch match;
				if(regex_search(source, match, namepat))
					name = match[1].str();
			}
			
			regex pat{"cat_([[:alnum:]]+)[[:s:]]+(.*?)[[:s:]]+cat_([[:alnum:]]+)"};
			for(sregex_iterator p(source.begin(), source.end(), pat); p != sregex_iterator{}; p++){
				Category c((*p)[2], (*p)[1]);
				cats.push_back(c);
			}
		}
		
		
		void run(void){
			while(true){
				string query;
				cout << "Ask: ";
				getline(cin, query);
				if(query == "quit") break; 
				for(Category c : cats){
					string res = c.respond(query);
					if(!res.empty()){
						cout << res << endl;
						break;
					}
				}
			}
		}
		
		
		vector<Category> getCats(void){
			return cats;
		}
		
		string getName(void){ return name; }
};




int main(int argc, char *argv[]){
	/*
    string s = 	"name=MyRobot "
    			"cat_test "
    			"	# query=What is your name   #"
    			"	# query=What are you called #"
    			"	# reply=My name is MyRobot  #"
    			"	# reply=I am called MyRobot # "
    			"cat_test "
				
				"cat_greet "
				"	# query=Hello there #"
				"	# query=Hi          #"
				"	# query=Hello       #"
				"	# reply=Hi { if p < 0: print(\"Error\") }        #"
				"	# reply=Hello  { / some comment / }     # "
				"cat_greet "
				
				"cat_greet "
				"	# query=How are you      #"
				"	# query=How do you do    #"
				"	# query=How're you       #"
				"	# query=How was your day #"
				"	# reply=Cool. And you?   #"
				"	# reply=Fine. And you?   # "
				"cat_greet";
    Robot r(s);
	r.run();
	*/
	
	Parser p("num sum==50+(34-29)");
    
    return 0;
}
