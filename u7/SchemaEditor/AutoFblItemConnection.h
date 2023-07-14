#pragma once
#include "../../VFrame30/FblItemRect.h"
#include "../../VFrame30/Schema.h"

struct AutoFblConnectionProposition
{
	VFrame30::SchemaPoint from;
	VFrame30::SchemaPoint to;

	QRectF addButtonRect;
};

class AutoFblItemConnection
{
public:
	void setItems(const std::vector<SchemaItemPtr>& items);
	std::vector<AutoFblConnectionProposition> getPropositions() const;

private:
	std::vector<std::shared_ptr<VFrame30::FblItemRect>> m_items;
	std::shared_ptr<VFrame30::SchemaLayer> m_layer;
};