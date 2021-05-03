#include<bitset>

class Address{
    private:
    std::bitset<24> addr;
    public:
    Address();
    std::string addressToString();
    bool stringToAddress(std::string str);
};

Address::Address(){
    for (int i = 0; i < 24; i++){
        addr.set(i, 0);
    }
}

std::string Address::addressToString(){
    return addr.to_string();
}

bool Address::stringToAddress(std::string str){
    if(str.length() != 24 ){
        return false;
    }
    for(int i=0;i<str.length();i++){
        addr.set(i, (str[i]=='1' ? 1 : 0));
    }
    return true;
}

