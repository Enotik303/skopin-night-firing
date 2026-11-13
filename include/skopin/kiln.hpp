#pragma once

#include "skopin/ware.hpp"

#include <stdexcept>
#include <string>
#include <vector>

namespace skopin {

enum class Cone {
    Cold,
    Bisque,
    Glaze,
};

enum class Door {
    Closed,
    Open,
};

class KilnError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Kiln {
public:
    Kiln();

    [[nodiscard]] Cone cone() const;
    [[nodiscard]] Door door() const;
    [[nodiscard]] int stokes() const;

    [[nodiscard]] std::vector<Ware> stack() const;

    void open();
    void close();

    void fire();

    void cool();

    void load(const Ware& ware);

    void unload(const std::string& mark);

private:
    Cone cone_ = Cone::Cold;
    Door door_ = Door::Closed;
    int stokes_ = 0;
    std::vector<Ware> stack_;      
    std::vector<char> glazeSeen_;  
};

}  
