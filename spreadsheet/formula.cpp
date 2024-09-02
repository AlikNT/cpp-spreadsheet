#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <sstream>

using namespace std::literals;

std::ostream& operator<<(std::ostream& output, FormulaError fe) {
    return output << "#ARITHM!";
}

namespace {
class Formula : public FormulaInterface {
public:
    // Реализуйте следующие методы:
    explicit Formula(std::string expression) try
        : ast_(ParseFormulaAST(expression)) {
    } catch (const std::exception& exc) {
        throw exc;
    }
    [[nodiscard]] Value Evaluate(const SheetInterface& sheet) const override {
        try {
            return ast_.Execute(sheet);
        } catch (const FormulaError& formula_error) {
            return formula_error;
        }
    }
    [[nodiscard]] std::string GetExpression() const override {
        std::stringstream ss;
        ast_.FormulaAST::PrintFormula(ss);
        return ss.str();
    }

    [[nodiscard]] std::vector<Position> GetReferencedCells() const override {
        std::set<Position> unique_cells(std::begin(ast_.GetCells()), std::end(ast_.GetCells()));
        return std::vector<Position>{unique_cells.begin(), unique_cells.end()};
    }

private:
    FormulaAST ast_;
};
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    try {
        return std::make_unique<Formula>(std::move(expression));
    } catch (const std::exception& e) {
        throw FormulaException(e.what());
    }
}