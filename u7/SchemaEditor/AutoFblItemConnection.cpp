#include "AutoFblItemConnection.h"


void AutoFblItemConnection::setItems(const std::vector<SchemaItemPtr>& items)
{
	m_items.clear();
	m_layer.reset();

	for (const auto& item : items)
	{
		auto fbl = std::dynamic_pointer_cast<VFrame30::FblItemRect>(item);
		if (fbl != nullptr)
		{
			m_items.push_back(fbl);

			Q_ASSERT(m_layer == nullptr || m_layer == item->parentLayer());
			m_layer = item->parentLayer();
		}
	}

	return;
}

std::vector<AutoFblConnectionProposition> AutoFblItemConnection::getPropositions() const
{
	std::vector<AutoFblConnectionProposition> result;

	if (m_items.size() < 2)
	{
		return result;
	}

	Q_ASSERT(m_layer);
	
	auto schema = m_layer->parentSchema();
	Q_ASSERT(schema);

	// "To" items are the items with furthest inputs.
	//
	VFrame30::SchemaPoint furthestInputPoint{-1, -1};

	for (const auto& item : m_items)
	{
		for (const auto& input : item->inputs())
		{
			if (furthestInputPoint.X < input.point().X)
			{
				furthestInputPoint = input.point();
			}
		}
	}

	if (furthestInputPoint.X < 0)
	{
		return result;
	}

	std::vector<std::shared_ptr<VFrame30::FblItemRect>> toItems;
	toItems.reserve(m_items.size());

	std::copy_if(m_items.begin(), m_items.end(), std::back_inserter(toItems),
				 [furthestInputPoint, schema](const std::shared_ptr<VFrame30::FblItemRect>& item)
				 {
					 return item->inputsCount() != 0 &&
						    std::abs(item->inputs().front().point().X - furthestInputPoint.X) <= (schema->gridSize() * schema->pinGridStep());
				 });

	// Get all available output pins.
	//
	std::vector<std::shared_ptr<VFrame30::FblItemRect>> fromItems;
	fromItems.reserve(m_items.size() - toItems.size());

	std::copy_if(m_items.begin(), m_items.end(), std::back_inserter(fromItems), 
				 [&toItems](const std::shared_ptr<VFrame30::FblItemRect>& item)
				 {
					 return std::find(toItems.begin(), toItems.end(), item) == toItems.end();
				 });
	
	std::list<VFrame30::AfbPin> outPins;

	for (const auto& fromItem : fromItems)
	{
		for (const auto& output : fromItem->outputs())
		{
			int connectionCount = m_layer->GetPinPosConnectinCount(output.point());
			if (connectionCount <= 1)  // 1 is this pin itself.
			{
				outPins.push_back(output);
			}
		}
	}

	outPins.sort([](const VFrame30::AfbPin& a, const VFrame30::AfbPin& b)
				 {
					 return a.point().Y < b.point().Y;
				 });

	// Get all available input pins.
	//
	std::list<VFrame30::AfbPin> inputPins;

	for (const auto& toItem : toItems)
	{
		for (const auto& input : toItem->inputs())
		{
			int connectionCount = m_layer->GetPinPosConnectinCount(input.point());
			if (connectionCount <= 1)  // 1 is this pin itself.
			{
				inputPins.push_back(input);
			}
		}
	}

	inputPins.sort([](const VFrame30::AfbPin& a, const VFrame30::AfbPin& b)
				   {
					   return a.point().Y < b.point().Y;
				   });

	// Get possible connections.
	//
	auto outputIt = outPins.begin();
	auto inputIt = inputPins.begin();

	result.reserve(std::min(outPins.size(), inputPins.size()));

	while (outputIt != outPins.end() && inputIt != inputPins.end())
	{
		AutoFblConnectionProposition p;

		p.from = outputIt->point();
		p.to = inputIt->point();

		QRectF rect;
		rect.setLeft(std::min(p.from.X, p.to.X));
		rect.setTop(std::min(p.from.Y, p.to.Y));
		rect.setRight(std::max(p.from.X, p.to.X));
		rect.setBottom(std::max(p.from.Y, p.to.Y));

		const double sizeIn = 1.0 / 7.0; // in
		p.addButtonRect = QRectF{rect.center().x() - sizeIn / 2.0, rect.center().y() - sizeIn / 2.0, sizeIn, sizeIn};

		result.push_back(p);

		++outputIt;
		++inputIt;
	}

	// If there are too many different propositions and fbl items are far away from each other, then it becomes messy.
	// To avoid it we just clear the result if the distances differences (size of links) is above 1 inch.
	//
	double minDist = std::numeric_limits<double>::max();
	double maxDist = std::numeric_limits<double>::lowest();

	for (const auto& link : result)
	{
		double d = link.distance();
		minDist = std::min(minDist, d);
		maxDist = std::max(maxDist, d);
	};

	if (result.empty() == false && std::abs(maxDist - minDist) >= 1.0)
	{
		result.clear();
	}

	return result;
}