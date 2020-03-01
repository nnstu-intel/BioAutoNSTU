#include <fstream>
#include <iostream>
#include <users.hpp>

unsigned int id_generator(unsigned int initial_low_bound = 0) {
    static unsigned int id = 0;
    if (initial_low_bound) {
        id = initial_low_bound;
        return id;
    }

    return ++id;
}

User::User() {}

void User::embed(const std::vector<float> descriptor) {
    this->_descriptor.resize(descriptor.size());
    std::copy(descriptor.begin(), descriptor.end(), this->_descriptor.begin());
}

json User::toJSON() const {
    json result;
    result["firstname"] = this->_firstname;
    result["secondname"] = this->_secondname;
    if (!this->_patronymic.empty()) {
        result["patronymic"] = this->_patronymic;
    }
    result["passport"] = this->_passport;
    result["id"] = this->_id;
    if (this->_descriptor.size()) {
        result["descriptor"] = json(this->_descriptor);
    }
    return result;
}

void User::parseJSON(const json& source) {
    const json& firstname = source["firstname"];
    const json& secondname = source["secondname"];
    const json& patronymic = source["patronymic"];
    const json& passport = source["passport"];
    const json& id = source["id"];
    const json& descriptor = source["descriptor"];

    this->_firstname = firstname.get<std::string>();
    this->_secondname = secondname.get<std::string>();
    this->_passport = passport.get<std::string>();
    
    if (id.is_number()) {
        this->_id = id.get<unsigned int>();
    } else {
        this->_id = id_generator();
    }

    if (patronymic.is_string()) {
        this->_patronymic = patronymic.get<std::string>();
    }   

    if (descriptor.is_array()) {
        this->_descriptor = descriptor.get<std::vector<float>>();
    }
}

User::~User() {}

std::vector<User> read_users(const std::string& filename, const std::string& networkVersion) {
    std::ifstream users_file(filename, std::ios::in);
    if (users_file.is_open()) {
        try {
            std::stringstream buffer;
            buffer << users_file.rdbuf();
            json parsed_users = json::parse(buffer.str());
            if (parsed_users.at("networkVersion").get<std::string>() != networkVersion) {
                throw std::runtime_error(
                    std::string("Network version in the file distinguish from the current")
                );
            }

            std::vector<User> users;
            for (json& user: parsed_users.at("users")) {
                User user_instance;
                user_instance.parseJSON(user);
                users.push_back(user_instance);
            }

            std::cout << "Users have been read from the file " << filename << std::endl;
            users_file.close();
            return users;
        } catch (std::exception& ex) {
            users_file.close();
            std::cout << "Could not read users from the file " << filename << std::endl;
            std::cout << ex.what() << std::endl;
            return std::vector<User>();
        }    
    } else {
        std::cout << "Could not open file" << filename << " with users info" << std::endl;
        return std::vector<User>();
    }
}

void update_users(
    const std::vector<User>& users,
    const std::string& filename,
    const std::string& networkVersion
) {
    std::ofstream users_file(filename, std::ios::out);

    if (!users_file.is_open()) {
        std::cout << "Could open users file for write" << std::endl; 
        return;
    }

    try {
        json body = json::object();
        body["version"] = networkVersion;
        body["users"] = json::array();
        for (const User& user: users) {
            body["users"].push_back(user.toJSON());
        }
        users_file << body.dump();
    } catch (std::exception& ex) {
        std::cout << "Could not write users to file." << ex.what() << std::endl;
    }

    users_file.close();
    return;
}
