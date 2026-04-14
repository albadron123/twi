#include<stdio.h>
#include<stdlib.h>
#include<string>
#include<map>
#include<dirent.h>
#include<cstring>

// int  -- test_int.twi was detected
// bool -- result_int.twi also was detected
std::map<int, bool> testsDetected = {};


bool files_equal(const char *p1, const char *p2) {
    FILE *f1 = fopen(p1, "rb");
    FILE *f2 = fopen(p2, "rb");
        
    fseek(f1, 0, SEEK_END);
	fseek(f2, 0, SEEK_END);

    long size1 = ftell(f1);
    long size2 = ftell(f2);

    if (size1 != size2) {
        fclose(f1);
        fclose(f2);
        return false;
    }

    rewind(f1);
    rewind(f2);

    unsigned char buf1[size1];
    unsigned char buf2[size2];
 
	size_t n1 = fread(buf1, 1, size1, f1);
	fread(buf2, 1, size2, f2);


	fclose(f1);
	fclose(f2);

	if (memcmp(buf1, buf2, n1) != 0) {
		return false;
	}
    return true;
}

int main(int argc, char** argv) {
	bool verbose = false;
	if(argc > 1)
	{
		if(argc == 2 && strcmp(argv[1], "-v") == 0) {
			verbose = true;
		}
		else {
			printf("can't parse the arguments to the autotest program\n");
			return 0;
		}
		
	}
	//get the names of the tests:

	struct dirent **namelist;

	int resCount = scandir("./tests", &namelist, NULL, alphasort);
	if(resCount == -1) {
		printf("Error: can't scan the directory\n");
		return 0;
	}
	for(int i = 0; i < resCount; ++i)
	{
		std::string currentName = namelist[i]->d_name;
		int foundNameId = currentName.find("test_");
		int foundFmtId = currentName.find(".twi");
		if(foundNameId != -1 &&
		   foundFmtId != -1)
		{
			//almost found a test
			std::string theRest = currentName
				.erase(foundFmtId, 4)
				.erase(foundNameId, 5);
			try {
				if(std::to_string(std::stoi(theRest)) == theRest) {
					testsDetected[std::stoi(theRest)] = false;
				}
			} catch (const std::exception) {
				//nothing here
			};
			

		}
	}
	for(int i = 0; i < resCount; ++i)
	{
		std::string currentName = namelist[i]->d_name;
		int foundNameId = currentName.find("result_");
		int foundFmtId = currentName.find(".txt");
		if(foundNameId != -1 &&
		   foundFmtId != -1)
		{
			//almost found a test
			std::string theRest = currentName
				.erase(foundFmtId, 4)
				.erase(foundNameId, 7);
			try {
				if(std::to_string(std::stoi(theRest)) == theRest) {
					int num = std::stoi(theRest);
					if(testsDetected.find(num) != testsDetected.end()) {
						testsDetected[num] = true;	
					}
					else {
						printf("\033[01;1m\x1b[31mUNDETECTED TEST FOR:\x1b[0m result_%d.txt\n", 
							   num);
					}
				}
			} catch (const std::exception) {
				//nothing here
			};		

		}
	}

	for(auto& pair : testsDetected) {
		if(!pair.second) {	
			printf("\033[01;1m\x1b[31mUNDETECTED RESULT FOR:\x1b[0m test_%d.twi\n", 
				   pair.first);
		}	
	}



	for(auto& pair : testsDetected) {
		if(pair.second) {
			std::string test_name = "tests/test_";
			test_name += std::to_string(pair.first);
			test_name += ".twi";

			std::string test_result = "tests/result_";
			test_result += std::to_string(pair.first);
			test_result += ".txt";
		
			if(verbose) {
				system(("./twi " + test_name + "").c_str());
			}	
			system(("./twi " + test_name + " > autotest__.txt").c_str());
			if(verbose) {
				system("./interpreter/run interpreter/bytecode.b");
			}
			system("./interpreter/run interpreter/bytecode.b > autotest__.txt");

			bool equals = files_equal("autotest__.txt", test_result.c_str());
			if(equals) {
				printf("\033[01;1m\x1b[32mPASSED:\x1b[0m %s\n", test_name.c_str());
			}
			else {
				printf("\033[01;1m\x1b[31mFAILED:\x1b[0m %s\n", test_name.c_str());
			}
		}
	}

	return 0;
}
