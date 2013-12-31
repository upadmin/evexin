/*
 * Copyright (c) 2013 Kevin Smith
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#include <Eve-Xin/QtUI/QtSkillDelegate.h>

#include <QApplication>
#include <QPainter>
#include <QRect>
#include <QString>

#include <Eve-Xin/QtUI/QtSkillModel.h>

namespace EveXin {

void drawElidedText(QPainter* painter, const QRect& region, const QString& text, int flags = Qt::AlignTop) {
	QString adjustedText(painter->fontMetrics().elidedText(text, Qt::ElideRight, region.width(), Qt::TextShowMnemonic));
	painter->setClipRect(region);
	painter->drawText(region, flags, adjustedText.simplified());
	painter->setClipping(false);
}

QtSkillDelegate::QtSkillDelegate(QObject* parent) : QStyledItemDelegate(parent), nameFont_(QApplication::font()), infoFont_(QApplication::font()) {
	int infoFontSizeDrop = nameFont_.pointSize() >= 10 ? 2 : 0;
	infoFont_.setStyle(QFont::StyleItalic);
	infoFont_.setPointSize(nameFont_.pointSize() - infoFontSizeDrop);
}

QSize QtSkillDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const {
	if (!index.data(QtSkillModel::IsSkillRole).toBool()) {
		QStyledItemDelegate::sizeHint(option, index);
	}
	int verticalMargin = 2;
	QFontMetrics nameMetrics(nameFont_);
	QFontMetrics infoMetrics(infoFont_);
	int height = 2 * verticalMargin + nameMetrics.height() + infoMetrics.height();
	return QSize(150, height);
}

void QtSkillDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
	if (!index.data(QtSkillModel::IsSkillRole).toBool()) {
		QStyledItemDelegate::paint(painter, option, index);
		return;
	}
	QRect fullRegion(option.rect);
	if (option.state & QStyle::State_Selected) {
		painter->fillRect(fullRegion, option.palette.highlight());
		painter->setPen(option.palette.highlightedText().color());
	}
	painter->save();
	int margin = 2;
	QString name = index.data(Qt::DisplayRole).toString() + QString(" (%1x)").arg(index.data(QtSkillModel::SkillMultiplierRole).toInt());
	QString infoText = "Untrained";
	QVariant level = index.data(QtSkillModel::SkillLevelRole);
	if (level.isValid()) {
		infoText = QString("Level %1").arg(level.toInt());
	}
	QFontMetrics nameMetrics(nameFont_);
	painter->setFont(nameFont_);
	QRect textRegion(fullRegion.adjusted(margin, 0, 0, 0));
	int nameHeight = nameMetrics.height() + margin;
	QRect nameRegion(textRegion.adjusted(0, margin, 0, 0));

	drawElidedText(painter, nameRegion, name);

	painter->setFont(infoFont_);
	painter->setPen(QPen(QColor(160,160,160)));

	QRect infoTextRegion(textRegion.adjusted(0, nameHeight, 0, 0));
	drawElidedText(painter, infoTextRegion, infoText);

	painter->restore();
}

}
