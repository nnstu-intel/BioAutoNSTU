#ifndef PI_USERS_HPP
#define PI_USERS_HPP

#include <string>
#include <vector>
#include <json.hpp>

using nlohmann::json;

class User {
    private:
        unsigned int _id = 0;
        std::string _firstname;
        std::string _secondname;
        std::string _patronymic;
        std::string _passport;
        std::vector<float> _descriptor;
    public:
        User();
        void embed(const std::vector<float> descriptor);
        json toJSON() const;
        void parseJSON(const json& source);
        ~User();
};

std::vector<User> read_users(const std::string& filename, const std::string& networkVersion);
void update_users(const std::vector<User>& users, const std::string& filename, const std::string& networkVersion);

#endif