#include "sheet.h"

#include "cell.h"
#include "common.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <optional>

using namespace std::literals;

Sheet::~Sheet() = default;

void Sheet::SetCell(Position pos, std::string text) {
    CheckPositionIsValid(pos);
    ResizeIfNeeded(pos);
    if (!cells_[pos.row][pos.col]) {
        cells_[pos.row][pos.col] = std::make_unique<Cell>(*this);
    }
    cells_[pos.row][pos.col]->Set(std::move(text));
}

const CellInterface* Sheet::GetCell(Position pos) const {
    return const_cast<Sheet*>(this)->GetCell(pos);
}

CellInterface* Sheet::GetCell(Position pos) {
    CheckPositionIsValid(pos);
    if(static_cast<size_t>(pos.row) >= cells_.size() || static_cast<size_t>(pos.col) >= cells_[pos.row].size() || !cells_[pos.row][pos.col]) {
        return nullptr;
    }
    return cells_[pos.row][pos.col].get();
}

void Sheet::ClearCell(Position pos) {
    CheckPositionIsValid(pos);
    if (static_cast<size_t>(pos.row) < cells_.size() && static_cast<size_t>(pos.col) < cells_[pos.row].size()) {
        cells_[pos.row][pos.col]->Clear();
        if (cells_[pos.row][pos.col]->GetReferencedCells().empty()) {
            cells_[pos.row][pos.col].reset();
        }
    }
}

Size Sheet::GetPrintableSize() const {
    int max_row = 0, max_col = 0;

    for (int row = 0; static_cast<size_t>(row) < cells_.size(); ++row) {
        for (int col = 0; static_cast<size_t>(col) < cells_[row].size(); ++col) {
            if (cells_[row][col]) {
                max_row = std::max(max_row, row + 1);
                max_col = std::max(max_col, col + 1);
            }
        }
    }

    return {max_row, max_col};
}

std::ostream& operator<<(std::ostream & out, const CellInterface::Value & value) {
    std::visit(
        [&](const auto& x) {
            out << x;
        },
        value);
    return out;
}

void Sheet::PrintValues(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            if (auto cell = GetCell({row, col})) {
                output << cell->GetValue();
            }
        }
        output << '\n';
    }
}

void Sheet::PrintTexts(std::ostream& output) const {
    Size size = GetPrintableSize();
    for (int row = 0; row < size.rows; ++row) {
        for (int col = 0; col < size.cols; ++col) {
            if (col > 0) {
                output << '\t';
            }
            if (auto cell = GetCell({row, col})) {
                output << cell->GetText();
            }
        }
        output << '\n';
    }
}

Cell * Sheet::GetCellOrigin(Position pos) const {
    CheckPositionIsValid(pos);
    if(static_cast<size_t>(pos.row) >= cells_.size() || static_cast<size_t>(pos.col) >= cells_[pos.row].size() || !cells_[pos.row][pos.col]) {
        return nullptr;
    }
    return cells_[pos.row][pos.col].get();
}

void Sheet::CheckPositionIsValid(Position pos) const {
    if (!pos.IsValid()) {
        throw InvalidPositionException("Position is out of bounds");
    }
}

void Sheet::ResizeIfNeeded(Position pos) {
    if (static_cast<size_t>(pos.row) >= cells_.size()) {
        cells_.resize(pos.row + 1);
    }
    if (static_cast<size_t>(pos.col) >= cells_[pos.row].size()) {
        cells_[pos.row].resize(pos.col + 1);
    }
}

std::unique_ptr<SheetInterface> CreateSheet() {
    return std::make_unique<Sheet>();
}
