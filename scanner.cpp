#include"scanner.h"
#include"utils.h"


// These are the support methods for the scan(...)

inline bool is_alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

inline bool is_digit(char c) {
	return (c >= '0' && c <= '9');
}

// The main logic of the scanner

bool scan(const char * src, std::vector<Token>& out) {
	bool hasErrors = false;
	const char* p = src;
	int line = 0;
	while(*p != '\0') {
		switch(*p) {	
			case ' ':
			case '\t':
			case '\r':
				break;
			case '\n':
				++line;
				break;
			// one char tokens
			case '(':
				out.push_back({LEFT_PAREN, "", 0, line});
				break;	
			case ')':
				out.push_back({RIGHT_PAREN, "", 0, line});
				break;
			case '{':
				out.push_back({LEFT_BRACE, "", 0, line});
				break;
			case '}':
				out.push_back({RIGHT_BRACE, "", 0, line});
				break;
			case '.':
				out.push_back({DOT, "", 0, line});
				break;
			case ',':
				out.push_back({COMMA, "", 0, line});
				break;
			case ';':
				out.push_back({SEMICOLON, "", 0, line});
				break;
			case ':':
				out.push_back({COLON, "", 0, line});
				break;
			case '+':
				out.push_back({PLUS, "", 0, line});
				break;
			case '-':
				out.push_back({MINUS, "", 0, line});
				break;
			case '*':
				out.push_back({STAR, "", 0, line});
				break;
			case '/':
				//one line comment case
				if(*(p+1) == '/') {
					p+=2;
					while(*p != '\0' && *p != '\n')
						++p;
					--p;
				}
				//multiple lines comment case
				else if (*(p+1) == '*') {
					p+=2;
					while(!((*p == '\0') || 
						    (*(p+1) == '\0') || 
							(*p == '*' && *(p+1) == '/'))) {
						++p;
					}
					if(*p == '\0') --p;
					if(*p == '*') ++p;
				}
				else { 
				//slash case
				out.push_back({SLASH, "", 0, line});
				}
				break;
			//one or two char tokens
			case '!':
				if(*(p+1) == '=') {
					++p;
					out.push_back({BANG_EQUAL, "", 0, line});
				}
				else {
					out.push_back({BANG, "", 0, line});
				}
				break;
			case '=':
				if(*(p+1) == '=') {
					++p;
					out.push_back({EQUAL_EQUAL, "", 0, line});
				}
				else {
					out.push_back({EQUAL, "", 0, line});
				}
				break;	
			case '>':	
				if(*(p+1) == '=') {
					++p;
					out.push_back({GREATER_EQUAL, "", 0, line});
				}
				else {
					out.push_back({GREATER, "", 0, line});
				}
				break;
			case '<':
				if(*(p+1) == '=') {
					++p;
					out.push_back({LESS_EQUAL, "", 0, line});
				}
				else {
					out.push_back({LESS, "", 0, line});
				}
				break;
			//Literals
			case '"':
				{
					const char* start = p;
					do {
						++p;
					} while (!(*p == '"' || *p == '\0' || *p == '\n'));
					if(*p == '\0' || *p == '\n')
					{
						printf("LINE %d: String does not terminate\n", 
							   line);
						hasErrors = true;
					}	
					else
					{
						out.push_back({STRING, "", 0, line});
						int last_id = out.size()-1;
						out[last_id].content.assign(start+1, p-start-1);
					}

					if(*p == '\0') --p;
				}
				break;
			default:
				if(is_alpha(*p) || *p == '_') {
					const char* start = p;
					do {
						++p;
					} while(is_alpha(*p) || *p == '_' || is_digit(*p));
					out.push_back({IDENTIFIER, "", 0, line});
					int last_id = out.size()-1;
					out[last_id].content.assign(start, p-start);
					if(keywords.find(out[last_id].content) != keywords.end()) {
						out[last_id].type = keywords[out[last_id].content];
					}
					--p;
				}
				else if(is_digit(*p)) {
					const char * start = p;
					do {
						++p;
					} while(is_digit(*p));
					if (*(p) == '.' && is_digit(*(p+1))) {
						++p;	
						do {
							++p;
						} while(is_digit(*p));
					}		
					out.push_back({NUMBER, "", 0, line});
						int last_id = out.size()-1;
						out[last_id].content.assign(start, p-start);
						out[last_id].value = std::stod(out[last_id].content);
					--p;

				}
				else {
					printf("LINE %d: Unexpected character %c\n",line, *p);
					hasErrors = true;
				}
		}
		++p;	
	}

	out.push_back({_EOF, "", 0, line});
	return hasErrors;
}

