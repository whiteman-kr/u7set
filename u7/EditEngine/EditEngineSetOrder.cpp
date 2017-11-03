#include "EditEngineSetOrder.h"
#include "EditSchemaWidget.h"

namespace EditEngine
{

	SetOrderCommand::SetOrderCommand(EditSchemaView* schemaView,
									 SetOrder setOrder,
									 const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
									 std::shared_ptr<VFrame30::SchemaLayer> layer,
									 QScrollBar* hScrollBar,
									 QScrollBar* vScrollBar) :
		EditCommand(schemaView, hScrollBar, vScrollBar)
	{
		assert(schemaView != nullptr);
		assert(items.empty() == false);

		m_items = items;
		m_layer = layer;

		m_setOrder = setOrder;

		return;
	}

	bool SetOrderCommand::checkIfCommandChangesOrder(
			SetOrder /*setOrder*/,
			const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& /*items*/,
			const std::list<std::shared_ptr<VFrame30::SchemaItem>>& /*layerItems*/)
	{
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		return true;			// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
/*
		if (items.empty() == true ||
			layerItems.empty() == true)
		{
			return false;
		}

		switch (setOrder)
		{
		case SetOrder::BringToFront:
			{
				std::vector<std::shared_ptr<VFrame30::SchemaItem>> selectedItemsOrder;
				std::list<std::shared_ptr<VFrame30::SchemaItem>> newOrder;
				selectedItemsOrder.reserve(items.size());

				for (std::shared_ptr<VFrame30::SchemaItem> si : layerItems)
				{
					auto found = std::find(items.begin(), items.end(), si);

					if (found != items.end())
					{
						selectedItemsOrder.push_back(*found);
					}
					else
					{
						newOrder.push_back(si);
					}
				}

				assert(selectedItemsOrder.size() == items.size());

				for (auto si : selectedItemsOrder)
				{
					newOrder.push_back(si);
				}

				// Check the order
				//
				if (newOrder.size() != layerItems.size())
				{
					assert(newOrder.size() == layerItems.size());
					return false;
				}

				auto si1 = layerItems.begin();
				auto si2 = newOrder.begin();

				for (; si1 != layerItems.end() && si2 != newOrder.end(); si1++, si2++)
				{
					if (*si1 != *si2)
					{
						return true;	// order was changed
					}
				}

				return false;
			}
			break;
		case SetOrder::BringForward:
			return true;
			break;
		case SetOrder::SendToBack:
			return true;
			break;
		case SetOrder::SendBackward:
			return true;
			break;
		default:
			assert(false);
		}
*/
		return false;
	}

	void SetOrderCommand::executeCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect)
	{
		m_oldOrder = m_layer->Items;

		switch (m_setOrder)
		{
		case SetOrder::BringToFront:
			{
				std::vector<std::shared_ptr<VFrame30::SchemaItem>> selectedItemsOrder;
				std::list<std::shared_ptr<VFrame30::SchemaItem>> newOrder;
				selectedItemsOrder.reserve(m_items.size());

				for (std::shared_ptr<VFrame30::SchemaItem> si : m_layer->Items)
				{
					auto found = std::find(m_items.begin(), m_items.end(), si);

					if (found != m_items.end())
					{
						selectedItemsOrder.push_back(*found);
					}
					else
					{
						newOrder.push_back(si);
					}
				}

				assert(selectedItemsOrder.size() == m_items.size());

				// Add items to their new place
				//
				for (auto si : selectedItemsOrder)
				{
					newOrder.push_back(si);
				}

				// Add real order to the end of Layer->Items
				//
				m_layer->Items.clear();
				m_layer->Items = newOrder;
			}
			break;

		case SetOrder::BringForward:
			{
				struct SetOrderItemStruct
				{
					bool isSelected;
					std::shared_ptr<VFrame30::SchemaItem> item;
				};

				std::list<SetOrderItemStruct> newOrder;

				// --
				//
				for (auto i : m_layer->Items)
				{
					bool isSelected = std::find(m_items.begin(), m_items.end(), i) != m_items.end();
					newOrder.push_back(SetOrderItemStruct{isSelected, i});
				}

				// --
				//
				for (auto revCurrentIterator = newOrder.rbegin(); revCurrentIterator != newOrder.rend(); revCurrentIterator++)
				{
					if (revCurrentIterator == newOrder.rbegin())
					{
						continue;
					}

					SetOrderItemStruct current = *revCurrentIterator;

					if (current.isSelected == true)
					{
						// Move if next item is not selected
						//
						auto nextItemIterator = revCurrentIterator;
						nextItemIterator --;	// This is reverse iterator, so it will point on the next item
						SetOrderItemStruct nextItem = *nextItemIterator;

						if ((*nextItemIterator).isSelected == false)
						{
							*nextItemIterator  = current;
							*revCurrentIterator = nextItem;
						}

						continue;
					}
				}

				// Add real order to the end of Layer->Items
				//
				m_layer->Items.clear();

				for (auto i : newOrder)
				{
					m_layer->Items.push_back(i.item);
				}
			}
			break;
		case SetOrder::SendToBack:
			{
				std::vector<std::shared_ptr<VFrame30::SchemaItem>> selectedItemsOrder;
				selectedItemsOrder.reserve(m_items.size());

				std::list<std::shared_ptr<VFrame30::SchemaItem>> newOrder;

				for (std::shared_ptr<VFrame30::SchemaItem> si : m_layer->Items)
				{
					auto found = std::find(m_items.begin(), m_items.end(), si);

					if (found != m_items.end())
					{
						selectedItemsOrder.push_back(*found);
					}
					else
					{
						newOrder.push_back(si);
					}
				}

				assert(selectedItemsOrder.size() == m_items.size());

				// Add items to their new place
				//
				for (auto rit = selectedItemsOrder.rbegin(); rit != selectedItemsOrder.rend(); rit++)
				{
					newOrder.push_front(*rit);
				}

				// Add real order to the end of Layer->Items
				//
				m_layer->Items.clear();
				m_layer->Items = newOrder;
			}
			break;

		case SetOrder::SendBackward:
			{
				struct SetOrderItemStruct
				{
					bool isSelected;
					std::shared_ptr<VFrame30::SchemaItem> item;
				};

				std::list<SetOrderItemStruct> newOrder;

				// --
				//
				for (auto i : m_layer->Items)
				{
					bool isSelected = std::find(m_items.begin(), m_items.end(), i) != m_items.end();
					newOrder.push_back(SetOrderItemStruct{isSelected, i});
				}

				// --
				//
				for (auto currentIterator = newOrder.begin(); currentIterator != newOrder.end(); currentIterator++)
				{
					if (currentIterator == newOrder.begin())
					{
						continue;
					}

					SetOrderItemStruct current = *currentIterator;

					if (current.isSelected == true)
					{
						// Move if prev item is not selected
						//
						auto prevItemIterator = currentIterator;
						prevItemIterator--;
						SetOrderItemStruct nextItem = *prevItemIterator;

						if ((*prevItemIterator).isSelected == false)
						{
							*prevItemIterator  = current;
							*currentIterator = nextItem;
						}

						continue;
					}
				}

				// Add real order to the end of Layer->Items
				//
				m_layer->Items.clear();

				for (auto i : newOrder)
				{
					m_layer->Items.push_back(i.item);
				}
			}
			break;
		default:
			assert(false);
		}

		*itemsToSelect = m_items;

		return;
	}

	void SetOrderCommand::unExecuteCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect)
	{
		*itemsToSelect = m_items;

		assert(m_layer->Items.size() == m_oldOrder.size());

		m_layer->Items.clear();
		m_layer->Items = m_oldOrder;

		return;
	}

}
