#include <QtTest/QtTest>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include <TreeDS/tree>
#include <TreeDS/view>

using namespace std;
using md::n;

class ReadmeTest : public QObject {

    Q_OBJECT

    private slots:
    void test() {
        md::nary_tree<string> t; // Empty tree created
        QCOMPARE(t.size(), 0);

        {
            t.insert_over(t.begin(), "hello");
            std::stringstream result;
            result << "size: " << t.size() << " (" << *t.begin() << ")";
            QCOMPARE(result.str(), "size: 1 (hello)");
        }

        md::binary_tree<int> myTree(
            n(100)(              // Root node
                n(),             // Left child of root (it is empty: no child)
                n(200)(          // Right child of root
                    n(300)(      // Left child of 200
                        n(500),  // Left child of 300
                        n(600)), // Right child of 300
                    n(400))));   // Right child of 200

        myTree.insert_over(
            find(myTree.begin(), myTree.end(), 300), // Iterator to the (first) position of node 300
            n(-10)(
                n(-20),
                n(-30)(
                    n(-40))));

        md::binary_tree<int, md::policy::in_order> inOrderTree(move(myTree));
        //                   ^^^^^^^^^^^^^^^^^^^^ Default policy

        {
            std::stringstream cout;
            // In-order: 100, -20, -10, -40, -30, 200, 400,
            cout << "In-order: ";
            for (auto& value : inOrderTree) {
                cout << value << ", ";
            }
            QCOMPARE(cout.str(), "In-order: 100, -20, -10, -40, -30, 200, 400, ");
        }

        {
            std::stringstream cout;
            cout << "Post-order: ";
            auto it = inOrderTree.begin(md::policy::post_order());
            while (it != inOrderTree.end(md::policy::post_order())) {
                cout << *it++ << ", ";
            }
            QCOMPARE(cout.str(), "Post-order: -20, -40, -30, -10, 400, 200, 100, ");
        }

        {
            std::stringstream cout;
            md::binary_tree_view<int, md::policy::breadth_first> view(inOrderTree);
            cout << "Breadth-first: ";
            for (auto& value : view) {
                cout << value << ", ";
            }
            QCOMPARE(cout.str(), "Breadth-first: 100, 200, -10, 400, -20, -30, -40, ");
        }
    }
};

QTEST_MAIN(ReadmeTest);
#include "ReadmeTest.moc"
