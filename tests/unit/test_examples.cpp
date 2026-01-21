// Example test cases for VDL
#include <catch.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("JSON basic operations", "[json]") {
    using nlohmann::json;
    
    SECTION("json creation") {
        json j;
        j["name"] = "VDL";
        j["version"] = "1.0.0";
        
        REQUIRE(j["name"] == "VDL");
        REQUIRE(j["version"] == "1.0.0");
    }
    
    SECTION("json array operations") {
        json arr = json::array();
        arr.push_back("item1");
        arr.push_back("item2");
        arr.push_back("item3");
        
        REQUIRE(arr.size() == 3);
        REQUIRE(arr[0] == "item1");
    }
    
    SECTION("json serialization") {
        json j;
        j["nested"]["value"] = 42;
        
        std::string str = j.dump();
        REQUIRE(str.find("42") != std::string::npos);
    }
}

TEST_CASE("Simple arithmetic tests", "[basic]") {
    SECTION("addition") {
        REQUIRE(2 + 2 == 4);
    }
    
    SECTION("multiplication") {
        REQUIRE(3 * 4 == 12);
    }
    
    SECTION("division") {
        REQUIRE(10 / 2 == 5);
    }
}

TEST_CASE("String operations", "[basic]") {
    std::string str = "VDL Project";
    
    SECTION("length") {
        REQUIRE(str.length() == 11);
    }
    
    SECTION("substring") {
        REQUIRE(str.substr(0, 3) == "VDL");
    }
    
    SECTION("find") {
        REQUIRE(str.find("Project") != std::string::npos);
    }
}
