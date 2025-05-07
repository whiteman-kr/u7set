#include "ApertureRecord.h"
#include <CommonLib/ConstStrings.h>
#include "../UtilsLib/WUtils.h"

QString ApertureRecord::toString() const
{
	return QString("%1;%2;%3;%4\n").
		arg(signalID).
		arg(E::valueToString(apertureType)).
		arg(coarseAperture).
		arg(fineAperture);
}

void ApertureRecord::fromString(const QString& str)
{
	signalID.clear();

	QStringList fields = str.split(Separator::SEMICOLON, Qt::SkipEmptyParts);

	if (fields.size() != 4)
	{
		return;
	}

	bool ok = false;

	apertureType = E::stringToValue<E::ApertureType>(fields[1], &ok);

	if (ok == false)
	{
		return;
	}

	signalID = fields[0];
	coarseAperture = fields[2].toDouble();
	fineAperture = fields[3].toDouble();
}

void ApertureRecord::saveToProto(Network::ApertureRecord* ar) const
{
	TEST_PTR_RETURN(ar);

	ar->set_signalid(signalID.toStdString());
	ar->set_aperturetype(TO_INT(apertureType));
	ar->set_coarseaperture(coarseAperture);
	ar->set_fineaperture(fineAperture);
}

void ApertureRecord::readFromProto(const Network::ApertureRecord& ar)
{
	signalID = QString::fromStdString(ar.signalid());
	apertureType = static_cast<E::ApertureType>(ar.aperturetype());
	coarseAperture = ar.coarseaperture();
	fineAperture = ar.fineaperture();
}

