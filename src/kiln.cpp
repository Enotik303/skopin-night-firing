#include "skopin/kiln.hpp"

namespace skopin {

Kiln::Kiln() = default;

Cone Kiln::cone() const {
    return cone_;
}

Door Kiln::door() const {
    return door_;
}

int Kiln::stokes() const {
    return stokes_;
}

std::vector<Ware> Kiln::stack() const {
    return stack_;
}

void Kiln::open() {
    if (door_ == Door::Open) {
        throw KilnError("door is already open");
    }
    if (cone_ == Cone::Glaze) {
        throw KilnError("too hot to open the door");
    }
    door_ = Door::Open;
}

void Kiln::close() {
    if (door_ == Door::Closed) {
        throw KilnError("door is already closed");
    }
    door_ = Door::Closed;
}

void Kiln::fire() {
    if (door_ != Door::Closed) {
        throw KilnError("door must be closed");
    }
    if (cone_ == Cone::Glaze) {
        throw KilnError("already at the top cone");
    }
    for (const Ware& ware : stack_) {
        if (ware.thickness_mm > kCrackThicknessMm) {
            throw KilnError("ware is too thick");
        }
    }
    if (cone_ == Cone::Bisque) {
        for (const Ware& ware : stack_) {
            if (ware.program == Program::Bisque) {
                throw KilnError("bisque ware would crack at the glaze cone");
            }
        }
    }

    cone_ = (cone_ == Cone::Cold) ? Cone::Bisque : Cone::Glaze;
    ++stokes_;
    if (cone_ == Cone::Glaze) {
        for (char& seen : glazeSeen_) {
            seen = 1;
        }
    }
}

void Kiln::cool() {
    if (door_ != Door::Closed) {
        throw KilnError("door must be closed");
    }
    if (cone_ == Cone::Cold) {
        throw KilnError("already cold");
    }
    if (cone_ == Cone::Bisque) {
        for (std::size_t i = 0; i < stack_.size(); ++i) {
            if (stack_[i].program == Program::Glaze && !glazeSeen_[i]) {
                throw KilnError("glaze has not reached the glaze cone yet");
            }
        }
    }

    cone_ = (cone_ == Cone::Glaze) ? Cone::Bisque : Cone::Cold;
    ++stokes_;
}

void Kiln::load(const Ware& ware) {
    if (door_ != Door::Open) {
        throw KilnError("door must be open");
    }
    if (cone_ == Cone::Glaze) {
        throw KilnError("cannot load at the glaze cone");
    }
    if (cone_ == Cone::Bisque && ware.program != Program::Glaze) {
        throw KilnError("only glaze ware can be loaded hot");
    }
    if (validateWare(ware).has_value()) {
        throw KilnError("invalid ware");
    }
    for (const Ware& inside : stack_) {
        if (inside.mark == ware.mark) {
            throw KilnError("mark is already inside");
        }
    }
    if (ware.kind == WareKind::Sagger && !stack_.empty()) {
        throw KilnError("sagger needs an empty kiln");
    }
    if (!stack_.empty() && stack_.front().kind == WareKind::Sagger) {
        throw KilnError("kiln is occupied by a sagger");
    }
    if (!fitsInKiln(stack_, ware)) {
        throw KilnError("ware does not fit");
    }

    stack_.push_back(ware);
    glazeSeen_.push_back(0);
}

void Kiln::unload(const std::string& mark) {
    if (door_ != Door::Open) {
        throw KilnError("door must be open");
    }
    if (stack_.empty()) {
        throw KilnError("kiln is empty");
    }
    if (stack_.back().mark != mark) {
        throw KilnError("not the top of the stack");
    }
    if (stack_.back().program == Program::Glaze && cone_ != Cone::Cold) {
        throw KilnError("glaze ware is removed only from a cold kiln");
    }

    stack_.pop_back();
    glazeSeen_.pop_back();
}

}  
