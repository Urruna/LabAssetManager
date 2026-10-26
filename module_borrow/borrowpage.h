#ifndef BORROWPAGE_H
#define BORROWPAGE_H

#include <QWidget>
class QStandardItemModel;


namespace Ui {
class BorrowPage;
}

class BorrowPage : public QWidget
{
    Q_OBJECT

public:
    explicit BorrowPage(QWidget *parent = 0);
    ~BorrowPage();

public slots:
	void refreshData();//约定的刷新窗口

signals:
	void dataChanged();

private slots:
	void on_btn_search_clicked();
	void on_btn_borrow_clicked();
	void on_btn_return_clicked();
	void on_btn_export_borrow_clicked();

private:
    Ui::BorrowPage *ui;
    QStandardItemModel *borrowModel;
};

#endif // BORROWPAGE_H
