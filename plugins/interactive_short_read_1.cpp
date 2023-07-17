#include "lgraph/lgraph.h"
#include "snb_common.h"
#include "snb_constants.h"

extern "C" bool Process(lgraph_api::GraphDB& db, const std::string& request, std::string& response) {
    // For breakdown
    static size_t call_ID = 0;
    call_ID++;
    std::string transaction_name = __FILE__;
    transaction_name = transaction_name.substr(0, transaction_name.length() - 4);
    std::string log = transaction_name;
    lgraph_api::set_call_desc(log);
    lgraph_api::set_call_id(call_ID);
    log = "start";
    lgraph_api::log_breakdown(log);

    
    std::string input = lgraph_api::base64::Decode(request);
    std::stringstream iss(input);
    int64_t person_id = ReadInt64(iss);

    auto txn = db.CreateReadTxn();
    std::stringstream oss;

    auto person = txn.GetVertexByUniqueIndex(PERSON, PERSON_ID, lgraph_api::FieldData::Int64(person_id));
    WriteString(oss, person[PERSON_FIRSTNAME].string());
    WriteString(oss, person[PERSON_LASTNAME].string());
    WriteInt64(oss, person[PERSON_BIRTHDAY].integer());
    WriteString(oss, person[PERSON_LOCATIONIP].string());
    WriteString(oss, person[PERSON_BROWSERUSED].string());
    auto place = txn.GetVertexIterator(person[PERSON_PLACE].integer());
    WriteInt64(oss, place[PLACE_ID].integer());
    WriteString(oss, person[PERSON_GENDER].string());
    WriteInt64(oss, person[PERSON_CREATIONDATE].integer());

    response = oss.str();
    return true;
}
