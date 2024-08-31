#include "cell.h"

#include <cassert>
#include <iostream>
#include <string>
#include <optional>
#include <regex>
#include <utility>

#include "sheet.h"

using namespace std::literals;

Cell::Cell(Sheet &sheet)
    : impl_(std::make_unique<EmptyImpl>())
    , sheet_(sheet) {
}

Cell::~Cell() = default;

void Cell::Set(std::string text) {
    if (text.empty()) {
        impl_ = std::make_unique<EmptyImpl>();
    } else if (text[0] == '\'') {
        // Если текст начинается с апострофа, то он трактуется как текст
        impl_ = std::make_unique<TextImpl>(text);
    } else if (text[0] == '=' && text.length() > 1) {
        // Если текст начинается с '=', то это формула (если текст не равен просто '=')
        std::unique_ptr<FormulaImpl> impl_tmp_ = std::make_unique<FormulaImpl>(text.substr(1), sheet_);
        AddDependencies(impl_tmp_.get());
        if (IsCircularDependency()) {
            RemoveDependencies();
            throw CircularDependencyException("Circular dependency"s);
        }
        impl_ = std::move(impl_tmp_);
        CacheInvalidate();
    } else {
        // В остальных случаях текстовая ячейка
        impl_ = std::make_unique<TextImpl>(std::move(text));
    }
}

void Cell::Clear() {
    RemoveDependencies();
    value_cache_.reset();
    CacheInvalidate();
    impl_ = std::make_unique<EmptyImpl>();
}

bool Cell::IsEmpty() const {
    return dynamic_cast<EmptyImpl*>(impl_.get());
}

Cell::Value Cell::GetValue() const {
    CellInterface::Value result = impl_->GetValue();
    if (dynamic_cast<FormulaImpl*>(impl_.get())) {
        if (value_cache_.has_value()) {
            return *value_cache_;
        }
        value_cache_ = result;
    }
    return result;
}
std::string Cell::GetText() const {
    return impl_->GetText();
}

std::vector<Position> Cell::GetReferencedCells() const {
    if (auto impl = dynamic_cast<FormulaImpl*>(impl_.get())) {
        return impl->GetFormula()->GetReferencedCells();
    }
    return {};
}

CellInterface::Value Cell::EmptyImpl::GetValue() const {
    return "";
}

std::string Cell::EmptyImpl::GetText() const {
    return "";
}

Cell::TextImpl::TextImpl(std::string text)
    : text_(std::move(text)) {
}

CellInterface::Value Cell::TextImpl::GetValue() const {
    if (text_[0] == '\'') {
        return text_.substr(1);
    }
    // Регулярное выражение для числа с плавающей запятой или целого числа
    static const std::regex number_regex(R"(^[-+]?\d*\.?\d+([eE][-+]?\d+)?$)");

    if (std::regex_match(text_, number_regex)) {
        // Если строка соответствует числовому формату, преобразуем её в double
        return std::stod(text_);
    }

    // Если строка не является числом, возвращаем её как есть
    return text_;
}

std::string Cell::TextImpl::GetText() const {
    return text_;
}

Cell::FormulaImpl::FormulaImpl(std::string expression, Sheet& sheet)
    : formula_(ParseFormula(std::move(expression)))
    , sheet_(sheet) {
}

CellInterface::Value Cell::FormulaImpl::GetValue() const {
    auto value = formula_->Evaluate(sheet_);

    if (std::holds_alternative<double>(value)) {
        return std::get<double>(value);
    } else if (std::holds_alternative<FormulaError>(value)) {
        return std::get<FormulaError>(value);
    }
    return {};
}

std::string Cell::FormulaImpl::GetText() const {
    return "=" + formula_->GetExpression();
}

FormulaInterface * Cell::FormulaImpl::GetFormula() const {
    return formula_.get();
}

void Cell::AddDependencies(const FormulaImpl *formula_impl) {
    FormulaInterface* formula = formula_impl->GetFormula();
    if (!formula->GetReferencedCells().empty()) {
        for (const auto& pos : formula->GetReferencedCells()) {
            if (!sheet_.GetCell(pos)) {
                sheet_.SetCell(pos, ""s);
            }
            Cell* to = sheet_.GetCellOrigin(pos);
            to_.insert(to);
            to->from_.insert(this);
        }
    }
}

void Cell::RemoveDependencies() {
    /*for (const auto to: to_) {
        to->from_.erase(this);
    }*/
    to_.clear();
}

bool Cell::IsCircularDependency() const {
    std::unordered_set<const Cell*> visited;
    return IsCircularDependencyDFS(this, visited);
}

bool Cell::IsCircularDependencyDFS(const Cell *cell, std::unordered_set<const Cell *> &visited) const {
    if (visited.count(cell)) {
        return true;
    }
    visited.insert(cell);
    for (const auto& dependent_cell : cell->to_) {
        if (IsCircularDependencyDFS(dependent_cell, visited)) {
            return true;
        }
    }
    visited.erase(cell);
    return false;
}

void Cell::CacheInvalidate() {
    value_cache_.reset();
    for (Cell* dependent_cell : from_) {
        dependent_cell->CacheInvalidate();
    }
}
