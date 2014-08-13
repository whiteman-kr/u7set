#ifndef FBLCONNECTIONSIMPL_H
#define FBLCONNECTIONSIMPL_H

#include "VideoItem.h"

namespace VFrame30
{/*
	enum ConnectionDirrection
	{
		Input,
		Output
	};

	// CConnectionPoint
	//
	class CConnectionPoint
	{
	private:
		CConnectionPoint()
		{
			CoCreateGuid(&guid);
		};
	public:
		CConnectionPoint(double x, double y, ConnectionDirrection dirrection, const GUID& guid)
		{
			this->point.X = x;
			this->point.Y = y;
			this->dirrection = dirrection;
			this->guid = guid;
		};
		CConnectionPoint(const Proto::ConnectionPoint& cpm)
		{
			LoadData(cpm);
		}

		// Other
		//

		///<summary> 
		/// Вычисление координат точки, для прямоугольного Fbl элемента
		///</summary>
		void CalcPointPos(const CDoubleRect& fblItemRect, int pinCount, int index, double minFblGridSize, VideoItemPoint* pResult) const
		{
			if (pResult == nullptr)
			{
				ASSERT(pResult);
				return;
			}

			double x = GetDirrection() == ConnectionDirrection::Input ? fblItemRect.left : fblItemRect.right;

			// вертикальное расстояние между пинами
			//
			double pinVertGap = fblItemRect.Height() / pinCount;

			double y = pinVertGap / 2 + pinVertGap * index;
			y = floor(y / minFblGridSize) * minFblGridSize;			// выровнять по сетке

			pResult->X = x;
			pResult->Y = y;
			return;
		}

		// Serialization
		//
		bool SaveData(Proto::ConnectionPoint& cpm) const
		{
			point.SaveData(*cpm.mutable_point());
			cpm.set_dirrection(static_cast<Proto::ConnectionDirrection>(GetDirrection()));
			VFrame30::Proto::Write(cpm.mutable_guid(), guid);
			return true;
		}
		bool LoadData(const Proto::ConnectionPoint& cpm)
		{
			point.LoadData(cpm.point());
			dirrection = static_cast<ConnectionDirrection>(cpm.dirrection());
			guid = VFrame30::Proto::Read(cpm.guid());
			return true;
		}

		// Properties
		//
	public:
		double GetX() const
		{
			return point.X;
		}
		void SetX(double val)
		{
			point.X = val;
		}

		double GetY() const
		{
			return point.Y;
		}
		void SetY(double val)
		{
			point.Y = val;
		}

		ConnectionDirrection GetDirrection() const
		{
			return dirrection;
		}

		const GUID& GetGuid() const
		{
			return guid;
		}
		void SetGuid(const GUID& guid)
		{
			this->guid = guid;
		}
	
		// Data
		//
	private:
		GUID guid;
		VideoItemPoint point;
		ConnectionDirrection dirrection;
	};
	*/

	// IFblConnections
	//
	/*class AFX_EXT_CLASS IFblConnections
	{
	public:
		virtual const list<CConnectionPoint>& GetInputs() const = NULL;
		virtual const list<CConnectionPoint>& GetOutputs() const = NULL;

		virtual bool GetNeighbourPoint(double x, double y, double maxDistance, CConnectionPoint& dstPoint) const = NULL;
	};*/


	// СFblConnectionsImpl
	//
	/*
	class AFX_EXT_CLASS CFblConnectionsImpl : public IFblConnections
	{
	public:
		virtual ~CFblConnectionsImpl()
		{
		}

		// Serialization
		//
	protected:
		virtual bool SaveData(VFrame30::Proto::Envelope& message) const;
		virtual bool LoadData(const VFrame30::Proto::Envelope& message);

	public:
		virtual const list<CConnectionPoint>& GetInputs() const;
		virtual const list<CConnectionPoint>& GetOutputs() const;

		virtual bool GetNeighbourPoint(double x, double y, double maxDistance, CConnectionPoint& dstPoint) const;

		bool GetConnectionPoint(const GUID& guid, CConnectionPoint* pResult) const;

		int GetInputsCount() const;
		int GetOutputsCount() const;

	protected:
		void AddInput();
		void AddOutput();

	private:
		list<CConnectionPoint> inputPoints;
		list<CConnectionPoint> outputPoints;
	};	
	*/
}

#endif