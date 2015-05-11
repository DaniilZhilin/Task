/*
Search Engin
SPbSU, Faculty of Applied Mathematics and Control Processes, 1st year
Zhilin Daniil
*/

#include <sstream>
#include <iostream>
#include <string>
#include <set>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <fstream>
#include <stdio.h>
#include "sha256.h"
#include <exception>


using namespace std;

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;
std::string test;

// Information of files in directory
struct Fileinfo {
	std::string path;
	std::string hash;
	int size;
	string flag;
};

// Checking of files in new directory. Comparison with result.json.
vector<Fileinfo> compare_lists(vector<Fileinfo> newfl, vector<Fileinfo> oldfl) {
	
	//Count up amount of strings in json file.
	//There are 3 strings for each file in result.json (for path, size and hash) and 3 empty strings;
	//Then counter = (all strings - 3 empty strings)/3;
	char *str = new char[1024];
	int counter = 0;
	ifstream base("result.json");
	while (!base.eof())
	{
		base.getline(str, 1024, '\n');
		counter++;
	}
	base.close();
	delete str;
	counter = (counter - 3) / 3;
	cout << counter << '\n';
	
	//Creat a tree and fill out information from result.json
	boost::property_tree::ptree tree;
	read_json("result.json", tree);
	string value;
	Fileinfo it1;

	//Fill out vector with this information
	for (int i = 1; counter;){
		value = tree.get<string>("File" + std::to_string(i));
		it1.path = value;
		value = tree.get<string>("Hash" + std::to_string(i));
		it1.hash = value;
		oldfl.push_back(it1);
		++i;
		if (i > counter){ 
			break;
		}
	}


	//Compare
	for (vector<Fileinfo>::iterator itnew = newfl.begin(); itnew < newfl.end(); itnew++) {
		for (vector<Fileinfo>::iterator itold = oldfl.begin(); itold < oldfl.end(); itold++) {
			if (itnew->hash == itold->hash) {
				itnew->flag = "UNCHANGED";
				oldfl.erase(itold);
				break;
			}
			if (itnew->hash != itold->hash) {
				itnew->flag = "CHANGED";
				oldfl.erase(itold);
				break;
			}
		}
	}
	for (vector<Fileinfo>::iterator itold = oldfl.begin(); itold < oldfl.end(); itold++) {
		itold->flag = "DELETED";
		newfl.push_back(*itold);
	}

	//Output
	for (Fileinfo data : newfl){
		cout << data.path << endl
	 	   	<< data.size << endl
			<< data.hash << endl
			<< data.flag << endl;
	}

	return newfl;
}

// Saving of kson file
void savejson(string filename, vector<Fileinfo> vec_finfo) {

	boost::property_tree::ptree tree;

	int i = 1;
	for (Fileinfo it : vec_finfo) {
		tree.put("File" + std::to_string(i), it.path);
		tree.put("Size" + std::to_string(i), it.size);
		tree.put("Hash" + std::to_string(i), it.hash);
		i++;
	}

	std::stringstream ss;
	boost::property_tree::write_json(ss, tree);
	const std::string result = ss.str();
	cout << result << endl;
	std::ofstream ofs("result.json");
	boost::property_tree::write_json(ofs, tree, true);
	ofs.close();
}

//Getting directory list with the help of boost filesystem
void get_dir_list(fs::directory_iterator iterator, std::vector<Fileinfo> * vec_finfo) {  
	Fileinfo finfo; 
	for (; iterator != fs::directory_iterator(); ++iterator)
	{

		if (fs::is_directory(iterator->status())) { 
			fs::directory_iterator sub_dir(iterator->path());
			get_dir_list(sub_dir, vec_finfo);

		}
		else 
		{
			
			//File in string
				stringstream result;
				string a;
				ifstream myfile;
				myfile.open(iterator->path().string(), ios::binary);
				result << myfile.rdbuf();
				a = result.str();

				//Hash file
				SHA256 sha256;
				string output1 = sha256(a);


				//Get information and put in vector
			finfo.path = iterator->path().string();
			finfo.size = fs::file_size(iterator->path());
			finfo.hash = output1;
			finfo.flag = "NEW";
			vec_finfo->push_back(finfo);
		}
		 
	}
}


	
int main() {

	string path, dirpath;
	
	cout << "What do you want?" << endl <<
		"press 1 - save new data" << endl << "press 2 - check new data" << endl;
	string status; // 1 or 2
	getline(cin, status);

	cout << "Folder path:" << std::endl;
	getline(cin, path);

	//If path doesn't exists stop the programm
	if (!(fs::exists(path))) {
			cout << "This directory doesn't exist. Please, try again.";
			status = "null";

		}

	else{
		std::vector<Fileinfo> vec_finfo;
		fs::directory_iterator home_dir(path);
		get_dir_list(home_dir, &vec_finfo);


		if (status == "1"){
			cout << endl;

			savejson("result.json", vec_finfo);
			std::cin.clear();
			fflush(stdin);

		}


		if (status == "2") {
			vector<Fileinfo> old;
			compare_lists(vec_finfo, old);
		}
	}
cin.get();
return 0;
}

