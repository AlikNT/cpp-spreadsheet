#pragma once

#include "common.h"
#include "formula.h"

#include <functional>
#include <optional>
#include <unordered_set>

class Sheet;

class Cell : public CellInterface {
public:
    explicit Cell(Sheet& sheet);
    ~Cell() override;

    void Set(std::string text);
    void Clear();
    bool IsEmpty() const;

    Value GetValue() const override;
    std::string GetText() const override;
    std::vector<Position> GetReferencedCells() const override;

private:
    //можете воспользоваться нашей подсказкой, но это необязательно.
    class Impl {
    public:
        virtual ~Impl() = default;

        [[nodiscard]] virtual Value GetValue() const = 0;

        [[nodiscard]] virtual std::string GetText() const = 0;
    };

    class EmptyImpl final : public Impl {
    public:
        [[nodiscard]] Value GetValue() const override;

        [[nodiscard]] std::string GetText() const override;
    };

    class TextImpl final : public Impl {
    public:
        explicit TextImpl(std::string text);

        [[nodiscard]] Value GetValue() const override;

        [[nodiscard]] std::string GetText() const override;

    private:
        std::string text_;
    };

    class FormulaImpl final : public Impl {
    public:
        explicit FormulaImpl(std::string expression, Sheet &sheet);

        [[nodiscard]] Value GetValue() const override;

        [[nodiscard]] std::string GetText() const override;

        [[nodiscard]] FormulaInterface* GetFormula() const;

    private:
        std::unique_ptr<FormulaInterface> formula_;
        Sheet& sheet_;
    };

    void AddDependencies(const FormulaImpl *formula_impl);
    void RemoveDependencies();
    bool IsCircularDependency() const;
    bool IsCircularDependencyDFS(const Cell* cell, std::unordered_set<const Cell*>& visited) const;

    void CacheInvalidate();

    std::unordered_set<Cell*> to_;
    std::unordered_set<Cell*> from_;
    std::unique_ptr<Impl> impl_;
    Sheet& sheet_;
    mutable std::optional<CellInterface::Value> value_cache_;
};
