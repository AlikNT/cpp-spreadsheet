#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet final : public SheetInterface {
public:
    ~Sheet() override;

    void SetCell(Position pos, std::string text) override;

    [[nodiscard]] const CellInterface* GetCell(Position pos) const override;

    CellInterface* GetCell(Position pos) override;

    void ClearCell(Position pos) override;

    [[nodiscard]] Size GetPrintableSize() const override;

    void PrintValues(std::ostream& output) const override;

    void PrintTexts(std::ostream& output) const override;

    // Можете дополнить ваш класс нужными полями и методами

    [[nodiscard]] Cell* GetCellOrigin(Position pos) const;

private:
    // Можете дополнить ваш класс нужными полями и методами
    std::vector<std::vector<std::unique_ptr<Cell>>> cells_;
    void CheckPositionIsValid(Position pos) const;
    void ResizeIfNeeded(Position pos);
};

std::ostream& operator<<(std::ostream & out, const CellInterface::Value & value);