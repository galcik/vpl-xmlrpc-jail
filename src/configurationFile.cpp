/**
 * version:		$Id: configurationFile.cpp,v 1.4 2014-02-21 18:13:30 juanca Exp $
 * package:		Part of vpl-jail-system
 * copyright:	Copyright (C) 2009 Juan Carlos Rodríguez-del-Pino. All rights reserved.
 * license:		GNU/GPL, see LICENSE.txt or http://www.gnu.org/licenses/gpl-3.0.html
 **/
#include <regex.h>
#include "util.h"
#include "configurationFile.h"
#include "httpServer.h"
#include <fstream>
using namespace std;
void ConfigurationFile::parseConfigLine(ConfigData &data,const string &line){
	static bool init=false;
	static regex_t reg,comment;
	if(!init){
		regcomp(&reg, "^[ \\t]*([^ \\t]+)[ \\t]*=[ \\t]*([^#]*)", REG_EXTENDED);
		regcomp(&comment, "^[ \\t]*(\\#.*)?$", REG_EXTENDED);
		init=true;
	}
	regmatch_t match[3];
	int nomatch=regexec(&reg, line.c_str(),3, match, 0);
	if(nomatch == 0){
		string param=line.substr(match[1].rm_so,match[1].rm_eo-match[1].rm_so);
		string value=line.substr(match[2].rm_so,match[2].rm_eo-match[2].rm_so);
		param = Util::toUppercase(param);
		Util::trim(value);
		if(data.find(param) != data.end())
			data[param]+=" "+value;
		else
			data[param]=value;
	}else{
		nomatch=regexec(&comment, line.c_str(),3, match, 0);
		if(nomatch != 0)
			throw HttpException(internalServerErrorCode
					,"Incorrect config file: "+line);
	}
}

ConfigData ConfigurationFile::readConfiguration(string fileName, ConfigData defaultData){
	ConfigData data;
	ifstream file;
	file.open(fileName.c_str(),ifstream::in);
	if(file.fail())
		throw HttpException(internalServerErrorCode
				,"Config file can't be opened",fileName);
	while(!file.fail()){
		string line;
		getline(file,line);
		if(file.fail()) break;
		parseConfigLine(data,line);
	}
	for(ConfigData::iterator i=defaultData.begin();i != defaultData.end(); i++){
		string param=i->first;
		param=Util::toUppercase(param);
		if(data.find(param) == data.end())
			data[param]=i->second;
	}
	return data;
}

void ConfigurationFile::writeConfiguration(string fileName, ConfigData data){
	ofstream file;
	file.open(fileName.c_str(),ifstream::out);
	if(file.fail())
		throw HttpException(internalServerErrorCode
				,"Config file can't be write",fileName);
	file << "#Autogenerated Config file of VPL " << endl;
	for(ConfigData::iterator i=data.begin();i != data.end(); i++){
		string line=i->first+"="+i->second;
		file << line << endl;
	}
	file.close();
}