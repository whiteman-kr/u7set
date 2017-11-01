#ifndef EDITCONNECTIONLINE_H
#define EDITCONNECTIONLINE_H

#include "../VFrame30/FblItem.h"
#include "../lib/TypesAndEnums.h"


namespace VFrame30
{
	class SchemaItem;
	class SchemaLayer;
	class PosConnectionImpl;
	class CDrawParam;
}

class EditConnectionLine
{
public:
	enum EditMode
	{
		AddToEnd,
		AddToBegin,
		EditPoint,
		EditEdge,
		MoveToPin
	};

	enum  Dirrection
	{
		Horz,
		Vert
	};

	enum PreferedMovePointWay
	{
		Auto,
		HorzWay,
		VertCorner,
		HorzCorner
	};


public:
	EditConnectionLine() = delete;
	EditConnectionLine(const EditConnectionLine& that) = default;
	EditConnectionLine(std::shared_ptr<VFrame30::PosConnectionImpl> item, EditConnectionLine::EditMode mode);

	// Methods
	//
public:
	void clear();
	void clearExtensionPoints();

	void addBasePoint(const QPointF& pt);
	void addExtensionPoint(const QPointF& pt);
	void moveEndPointPos(std::shared_ptr<VFrame30::SchemaLayer> layer,
						 QPointF toPoint,
						 PreferedMovePointWay preferedWay,
						 double gridSize);

	void removeLastBasePoint();

	int moveExtensionPointsToBasePoints();

	std::list<QPointF> basePoints() const;
	std::list<QPointF> extensionPoints() const;
	std::vector<QPointF> points() const;				// Get all points depending on m_editDirrection

	QPointF lastBasePoint() const;
	QPointF lastExtensionPoint() const;

	void setPointToItem(std::shared_ptr<VFrame30::PosConnectionImpl> schemaItem) const;

	void drawOutline(VFrame30::CDrawParam* drawParam) const;

	EditConnectionLine::EditMode mode() const;
	void setMode(EditConnectionLine::EditMode value);

	// EditDirrection::EditPoint only
	//
	void modifyPoint(const QPointF& point);
	bool addPointAndSwitchMode(const QPointF& point);		// Add point to front or back depending on currentIndex, if point in the middle the return false and do nothing

	QPointF editPointCurrState() const;
	int editPointIndex() const;
	void setEditPointIndex(std::shared_ptr<VFrame30::PosConnectionImpl> schemaItem, int pointIndex);

	// EditDirrection::EditEdge only
	//
	void modifyEdge(double value);
	double editEdgetCurrState() const;
	int editEdgeIndex() const;
	void setEditEdgeIndex(std::shared_ptr<VFrame30::PosConnectionImpl> schemaItem, int edgeIndex);

	// EditDirrection::MoveToPin only
	//
	void moveToPin_init(std::shared_ptr<VFrame30::PosConnectionImpl> link,
					   VFrame30::ConnectionDirrection pinDirrection,
					   QPointF pinInitialPos);
	void moveToPin_offset(std::shared_ptr<VFrame30::SchemaLayer> layer,
				   QPointF offset,
				   double gridSize);
	std::shared_ptr<VFrame30::SchemaItem> moveToPin_schemaItem() const;
	void moveToPin_setMoveWholeLink();

	bool moveToPin_isInput() const;
	bool moveToPin_isOutput() const;

public:
	static Dirrection getDirrection(const QPointF& pt1, const QPointF& pt2);
	static bool isHorz(const QPointF& pt1, const QPointF& pt2);
	static bool isVert(const QPointF& pt1, const QPointF& pt2);

	static std::vector<VFrame30::SchemaPoint> removeUnwantedPoints(const std::vector<VFrame30::SchemaPoint>& source);
	static std::list<VFrame30::SchemaPoint> removeUnwantedPoints(const std::list<VFrame30::SchemaPoint>& source);

	// Data
	//
private:
	EditMode m_mode = AddToEnd;
	std::list<QPointF> m_basePoints;

	// EditDirrection::AddToEnd/AddToBegin only
	//
	std::list<QPointF> m_extensionPoints;

	// EditDirrection::EditPoint only
	//
	struct
	{
		int pointIndex = 0;						// EditDirrection::EditPoint
		std::vector<QPointF> initialState;		// Initial state for EditDirrection::EditPoint mode
		QPointF pointCurrState;
	} m_editPoint;

	// EditDirrection::EditEdge only
	//
	struct
	{
		int edgeIndex = 0;						// Edge consist of two points m_basePoints[m_editEdgePointIndex]  and m_basePoints[m_editEdgePointIndex + 1]
		std::vector<QPointF> initialState;		// Initial state for EditDirrection::EditEdge mode
		double edgeCurrState = 0;
	} m_editEdge;

	// EditDirrection::MoveToPin only
	//
	struct
	{
		std::shared_ptr<VFrame30::PosConnectionImpl> schemaItem;
		std::vector<QPointF> initialState;		// Initial state modified Link
		VFrame30::ConnectionDirrection pinDirrection = VFrame30::ConnectionDirrection::Input;
		QPointF pinInitialPos;
		bool moveLinkBack = false;				// if true, move end of link, if false move begion of link
		bool moveWholeLink = false;				// Do not try to calc while link, just move all its point to ooffset
	} m_moveToPin;

};



#endif // EDITCONNECTIONLINE_H
