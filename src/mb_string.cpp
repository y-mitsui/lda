#include <iostream>
#include <string.h>
#include <string>
#include <vector>
#include <sstream>

using namespace std;

string percentEnc(string str){
  const int NUM_BEGIN_UTF8 = 48;
  const int CAPITAL_BEGIN_UTF8 = 65;
  const int LOWER_BEGIN_UTF8 = 97;

  int charCode=-1;
  std::string encoded;
  std::stringstream out;

  //for文で1byteずつストリームに入れていく
  for(int i=0;str[i]!=0;i++){
    //文字列中の1byte分のデータを整数値として代入
    charCode = (int)(unsigned char)str[i];

    //エンコードする必要の無い文字の判定
    if((NUM_BEGIN_UTF8 <= charCode && charCode <= NUM_BEGIN_UTF8 + 9)
       || (CAPITAL_BEGIN_UTF8 <= charCode && charCode <= CAPITAL_BEGIN_UTF8 + 25)
       || (LOWER_BEGIN_UTF8 <= charCode && charCode <= LOWER_BEGIN_UTF8 + 25)
       || str[i] == '.' || str[i] == '_' || str[i] == '-' || str[i] == '~')
    {
      //エンコードの必要が無い文字はそのままストリームに入れる
      out<<str[i];
    }else{
      //エンコードする場合は%記号と文字コードの16進数表示をストリームに入れる
      out << '%' << std::hex << std::uppercase <<charCode;
    }
  }
  //ストリームの文字列をstringのインスタンスに代入しreturn
  encoded = out.str();
  return encoded;
}

typedef unsigned char u8;
/**
 * 文字のバイト数分スキップ
 */
int get_skip_chars(unsigned char chr)
{
    if((chr & 0x80) == 0x00) return 1;
    else if((chr & 0xe0) == 0xc0) return 2;
    else if((chr & 0xf0) == 0xe0) return 3;
    else if((chr & 0xf8) == 0xf0) return 4;
    else if((chr & 0xfc) == 0xf8) return 5;
    else if((chr & 0xfe) == 0xfc) return 6;
    return -1;
}

/**
 * UTF-8をUSC-4に変換
 */
unsigned int get_UCS4_code(const char *ptr, int* skip)
{
    const u8 *p = (const u8 *)ptr;
    unsigned int chr = (unsigned int)*p;
    if(chr < 0x80)
    {
        *skip = 1;
        return chr;
    }
    static const unsigned int base[] = {
        0x00000000, 0x00000000, 0x00003080, 0x000E2080,
        0x03C82080, 0xFA082080, 0x82082080};

    int size = get_skip_chars(chr);
    for(int i = 1; i < size; i++)
        chr = (chr << 6) + p[i];

    *skip = size;
    return chr - base[size];
}

/**
 * 日本語判定
 */
bool checkJapanese(const char* str)
{
    int skip;
    int count = 0;
    bool flag = false;

    while(*str)
    {
        skip = 0;

        unsigned int code = get_UCS4_code(str, &skip);
        if(code >= 0x3040 && code <= 0x309f)
            return true; //hiragana
        else if(code >= 0x30a0 && code <= 0x30ff)
            return true; //katakana
        else if(code >= 0x31F0 && code <= 0x31FF)
            return true; //sutekana
        else if(code >= 0xFF01 && code <= 0xFF9D)
            flag = true; //zenkaku-kigo
        else if(code >= 0x3000 && code <= 0x303F)
            flag = true; //kutouten
        else
            ++count;

        str += skip;
    }

    if (flag && count < 7) return true;

    return false;
}

void eachChar(std::string str, vector<string> &charactors){
    using namespace std;
    unsigned char lead;
    int char_size = 0;

    for (string::iterator it = str.begin(); it != str.end(); it += char_size) {
        lead = *it;
        if (lead < 0x80) {
            char_size = 1;
        } else if (lead < 0xE0) {
            char_size = 2;
        } else if (lead < 0xF0) {
            char_size = 3;
        } else {
            char_size = 4;
        }
        string charactor = str.substr(distance(str.begin(), it), char_size);
        charactors.push_back(charactor);
    }
}

unsigned int utf8_finish(char *str, int n_limit) {
    unsigned int len = 0;
    int pos;
    unsigned char lead; 
    int char_size = 0;
    int len_str = strlen(str);
    int n_moji;
    
    for (pos = 0, n_moji = 0; pos < len_str; pos += char_size, n_moji++) {
        if (n_moji == n_limit) {
            str[pos] = '\0';
            break;
        }
        
        lead = str[pos];

        if (lead < 0x80) {
            char_size = 1;
        } else if (lead < 0xE0) {
            char_size = 2;
        } else if (lead < 0xF0) {
            char_size = 3;
        } else {
            char_size = 4;
        }

        len += 1;
    }

    return len;
}

unsigned int utf8_strlen(std::string str)
{
    using namespace std;

    unsigned int len = 0;
    int pos;
    unsigned char lead; 
    int char_size = 0;

    for (pos = 0; pos < str.size(); pos += char_size) {

        lead = str[pos];

        if (lead < 0x80) {
            char_size = 1;
        } else if (lead < 0xE0) {
            char_size = 2;
        } else if (lead < 0xF0) {
            char_size = 3;
        } else {
            char_size = 4;
        }

        len += 1;
    }

    return len;
}

int zen2han(char *str) {
    char *buf,*p,*ptr;

	buf=(char *)calloc(strlen(str)+1,sizeof(char));
	for(ptr=str,p=buf;*ptr!='\0';*ptr++){
		if(strncmp(ptr,"　", 3)==0){*p=' ';p++;ptr+=2;}
		else if(strncmp(ptr,"！",3)==0){*p='!';p++;ptr+=2;}
		else if(strncmp(ptr,"”",3)==0){*p='"';p++;ptr+=2;}
		else if(strncmp(ptr,"＃",3)==0){*p='#';p++;ptr+=2;}
		else if(strncmp(ptr,"＄",3)==0){*p='$';p++;ptr+=2;}
		else if(strncmp(ptr,"％",3)==0){*p='%';p++;ptr+=2;}
		else if(strncmp(ptr,"＆",3)==0){*p='&';p++;ptr+=2;}
		else if(strncmp(ptr,"’",3)==0){*p='\'';p++;ptr+=2;}
		else if(strncmp(ptr,"（",3)==0){*p='(';p++;ptr+=2;}
		else if(strncmp(ptr,"）",3)==0){*p=')';p++;ptr+=2;}
		else if(strncmp(ptr,"＊",3)==0){*p='*';p++;ptr+=2;}
		else if(strncmp(ptr,"＋",3)==0){*p='+';p++;ptr+=2;}
		else if(strncmp(ptr,"，",3)==0){*p=',';p++;ptr+=2;}
		else if(strncmp(ptr,"－",3)==0){*p='-';p++;ptr+=2;}
		else if(strncmp(ptr,"．",3)==0){*p='.';p++;ptr+=2;}
		else if(strncmp(ptr,"／",3)==0){*p='/';p++;ptr+=2;}
		else if(strncmp(ptr,"０",3)==0){*p='0';p++;ptr+=2;}
		else if(strncmp(ptr,"１",3)==0){*p='1';p++;ptr+=2;}
		else if(strncmp(ptr,"２",3)==0){*p='2';p++;ptr+=2;}
		else if(strncmp(ptr,"３",3)==0){*p='3';p++;ptr+=2;}
		else if(strncmp(ptr,"４",3)==0){*p='4';p++;ptr+=2;}
		else if(strncmp(ptr,"５",3)==0){*p='5';p++;ptr+=2;}
		else if(strncmp(ptr,"６",3)==0){*p='6';p++;ptr+=2;}
		else if(strncmp(ptr,"７",3)==0){*p='7';p++;ptr+=2;}
		else if(strncmp(ptr,"８",3)==0){*p='8';p++;ptr+=2;}
		else if(strncmp(ptr,"９",3)==0){*p='9';p++;ptr+=2;}
		else if(strncmp(ptr,"：",3)==0){*p=':';p++;ptr+=2;}
		else if(strncmp(ptr,"；",3)==0){*p=';';p++;ptr+=2;}
		else if(strncmp(ptr,"＜",3)==0){*p='<';p++;ptr+=2;}
		else if(strncmp(ptr,"＝",3)==0){*p='=';p++;ptr+=2;}
		else if(strncmp(ptr,"＞",3)==0){*p='>';p++;ptr+=2;}
		else if(strncmp(ptr,"？",3)==0){*p='?';p++;ptr+=2;}
		else if(strncmp(ptr,"＠",3)==0){*p='@';p++;ptr+=2;}
		else if(strncmp(ptr,"Ａ",3)==0){*p='A';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｂ",3)==0){*p='B';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｃ",3)==0){*p='C';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｄ",3)==0){*p='D';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｅ",3)==0){*p='E';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｆ",3)==0){*p='F';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｇ",3)==0){*p='G';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｈ",3)==0){*p='H';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｉ",3)==0){*p='I';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｊ",3)==0){*p='J';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｋ",3)==0){*p='K';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｌ",3)==0){*p='L';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｍ",3)==0){*p='M';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｎ",3)==0){*p='N';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｏ",3)==0){*p='O';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｐ",3)==0){*p='P';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｑ",3)==0){*p='Q';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｒ",3)==0){*p='R';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｓ",3)==0){*p='S';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｔ",3)==0){*p='T';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｕ",3)==0){*p='U';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｖ",3)==0){*p='V';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｗ",3)==0){*p='W';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｘ",3)==0){*p='X';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｙ",3)==0){*p='Y';p++;ptr+=2;}
		else if(strncmp(ptr,"Ｚ",3)==0){*p='Z';p++;ptr+=2;}
		else if(strncmp(ptr,"［",3)==0){*p='[';p++;ptr+=2;}
		else if(strncmp(ptr,"￥",3)==0){*p='\\';p++;ptr+=2;}
		else if(strncmp(ptr,"］",3)==0){*p=']';p++;ptr+=2;}
		else if(strncmp(ptr,"＾",3)==0){*p='^';p++;ptr+=2;}
		else if(strncmp(ptr,"＿",3)==0){*p='_';p++;ptr+=2;}
		else if(strncmp(ptr,"‘",3)==0){*p='`';p++;ptr+=2;}
		else if(strncmp(ptr,"ａ",3)==0){*p='a';p++;ptr+=2;}
		else if(strncmp(ptr,"ｂ",3)==0){*p='b';p++;ptr+=2;}
		else if(strncmp(ptr,"ｃ",3)==0){*p='c';p++;ptr+=2;}
		else if(strncmp(ptr,"ｄ",3)==0){*p='d';p++;ptr+=2;}
		else if(strncmp(ptr,"ｅ",3)==0){*p='e';p++;ptr+=2;}
		else if(strncmp(ptr,"ｆ",3)==0){*p='f';p++;ptr+=2;}
		else if(strncmp(ptr,"ｇ",3)==0){*p='g';p++;ptr+=2;}
		else if(strncmp(ptr,"ｈ",3)==0){*p='h';p++;ptr+=2;}
		else if(strncmp(ptr,"ｉ",3)==0){*p='i';p++;ptr+=2;}
		else if(strncmp(ptr,"ｊ",3)==0){*p='j';p++;ptr+=2;}
		else if(strncmp(ptr,"ｋ",3)==0){*p='k';p++;ptr+=2;}
		else if(strncmp(ptr,"ｌ",3)==0){*p='l';p++;ptr+=2;}
		else if(strncmp(ptr,"ｍ",3)==0){*p='m';p++;ptr+=2;}
		else if(strncmp(ptr,"ｎ",3)==0){*p='n';p++;ptr+=2;}
		else if(strncmp(ptr,"ｏ",3)==0){*p='o';p++;ptr+=2;}
		else if(strncmp(ptr,"ｐ",3)==0){*p='p';p++;ptr+=2;}
		else if(strncmp(ptr,"ｑ",3)==0){*p='q';p++;ptr+=2;}
		else if(strncmp(ptr,"ｒ",3)==0){*p='r';p++;ptr+=2;}
		else if(strncmp(ptr,"ｓ",3)==0){*p='s';p++;ptr+=2;}
		else if(strncmp(ptr,"ｔ",3)==0){*p='t';p++;ptr+=2;}
		else if(strncmp(ptr,"ｕ",3)==0){*p='u';p++;ptr+=2;}
		else if(strncmp(ptr,"ｖ",3)==0){*p='v';p++;ptr+=2;}
		else if(strncmp(ptr,"ｗ",3)==0){*p='w';p++;ptr+=2;}
		else if(strncmp(ptr,"ｘ",3)==0){*p='x';p++;ptr+=2;}
		else if(strncmp(ptr,"ｙ",3)==0){*p='y';p++;ptr+=2;}
		else if(strncmp(ptr,"ｚ",3)==0){*p='z';p++;ptr+=2;}
		else if(strncmp(ptr,"｛",3)==0){*p='{';p++;ptr+=2;}
		else if(strncmp(ptr,"｜",3)==0){*p='|';p++;ptr+=2;}
		else if(strncmp(ptr,"｝",3)==0){*p='}';p++;ptr+=2;}
		else{ *p=*ptr; p++; }
	}
	strcpy(str,buf);
	free(buf);

	return(0);
}



int isKanji(unsigned int code) {
    return (code >= 0x4E00 && code <= 0x9FFF);
}
int isKana(unsigned int code) {
    return (code >= 0x30a0 && code <= 0x30ff);
}
int isHiragana(unsigned int code) {
    return (code >= 0x3040 && code <= 0x309f);
}

int isAvalilableCharactor(const char *str) {
    int skip;
    while(*str) {
        skip = 0;
        unsigned int code = get_UCS4_code(str, &skip);
        if (!isKanji(code) && !isKana(code) && !isHiragana(code) && code != 0x30FC && code != 0x2015 && code != 0x2010 && !isalnum(*str) && *str != '-')
            return 0;
        str += skip;
    }
    return 1;
}
