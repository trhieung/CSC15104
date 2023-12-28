#include "global.h"
#include "fileTemplate.h"
#include "fileCombine.h"

void gen_random_myStruct(myStruct& a) {
    // Function to generate a random string of fixed length (10 bytes)
    auto generate_random_string = []() -> std::string {
        const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, characters.size() - 1);

        std::string result;
        for (int i = 0; i < 10; ++i) {
            result += characters[dis(gen)];
        }

        return result;
    };

    // Generate random values for each member of myStruct
    a.mssv = generate_random_string();
    a.name = generate_random_string();
    a.birth_date = generate_random_string();
    a.join_date = generate_random_string();
    a.phone = generate_random_string();
    a.id_card = generate_random_string();
}

std::string convert_myStruct_to_string(myStruct a){
    return a.mssv + a.name + a.birth_date + a.join_date + a.phone + a.id_card;
}

int main() {
    std::cout << "hughu" << std::endl;
    // FileTemplate x("mystie", 1<<15, 1,"pass@word");
    // std::cout << "hughu" << std::endl;
    fileCombine myf("huhu", 1 <<15, {7, 1}, "huhuhu",
                    "haha", 0, "hahaha",
                    "hihi", 0, "hihihi");

    Student a;
    Teacher x;
    std::string m;
    gen_random_myStruct(a);
    gen_random_myStruct(x);
    m = convert_myStruct_to_string(a);

    // add information in combine file
    for(int i = 0; i < (1 << 6); i++){
        myf._add("haha", "hahaha", m);
    }

    for(int i = 0; i < (1 << 4); i++){    
        myf._add("hihi", "hihihi", m);
    }
    
    // change pwd
    if(!myf.change_pwd("haha", "hahaha", "hahi")) std::cout << "change pwd error" << std::endl;

    if(!myf._ls("haha", "hahaha")) 
        std::cout << "pwd not correct" << std::endl;

    if(myf._ls("haha", "hahi")) std::cout << "list file success" << std::endl;
    
    return 0;
}