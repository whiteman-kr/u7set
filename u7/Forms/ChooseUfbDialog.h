#pragma once

#include "../VFrame30/UfbSchema.h"

namespace Ui {
class ChooseUfbDialog;
}

class ChooseUfbDialog : public QDialog
{
	Q_OBJECT

public:
	explicit ChooseUfbDialog(const std::vector<std::shared_ptr<VFrame30::UfbSchema>>& ufbs, QWidget* parent = nullptr);
	~ChooseUfbDialog();

protected:
	virtual void showEvent(QShowEvent* event) override;

private:
	void fillTree();
	void itemSelectionChanged();
	void itemDoubleClicked(QModelIndex index);

public:
	std::shared_ptr<VFrame30::UfbSchema> result();

private:
	Ui::ChooseUfbDialog *ui;
	std::vector<std::shared_ptr<VFrame30::UfbSchema>> m_ufbs;

	std::shared_ptr<VFrame30::UfbSchema> m_selectedUfb;
};

