#include "excel.h"
#include <OpenXLSX/OpenXLSX.hpp>

using namespace utils;
using namespace OpenXLSX;

void utils::saveDataToXsl(const Json&) {
    XLDocument doc;
    doc.create("Spreadsheet.xlsx");
    auto wks = doc.workbook().worksheet("Sheet1");

    wks.cell("A1").value() = "Hello, OpenXLSX!";

    doc.save();
}