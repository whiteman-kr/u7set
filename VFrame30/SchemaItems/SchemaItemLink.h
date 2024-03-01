#pragma once

#include "PosConnectionImpl.h"
#include "FblItemLine.h"

namespace VFrame30
{
	class SchemaItemLink final : public FblItemLine
	{
		Q_OBJECT

	public:
		SchemaItemLink(void);
		explicit SchemaItemLink(SchemaUnit unit);
		virtual ~SchemaItemLink(void);

		// Serialization
		//
	protected:
		virtual bool SaveData(Proto::Envelope* message) const override;
		virtual bool LoadData(const Proto::Envelope& message) override;

		// Draw Functions
		//
	public:

		// Рисование элемента, выполняется в 100% масштабе.
		// Graphcis должен иметь экранную координатную систему (0, 0 - левый верхний угол, вниз и вправо - положительные координаты)
		//
		virtual void draw(CDrawParam* drawParam) const override;

		// Вычислить координаты точки
		//
		virtual void SetConnectionsPos(double gridSize, int pinGridStep) override;
		virtual bool GetConnectionPointPos(const QUuid& connectionPointGuid, SchemaPoint* pResult, double gridSize, int pinGridStep) const override;

		// Properties and Data
		//
	public:
	private:
	};
}
