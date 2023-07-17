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
    int64_t message_id = ReadInt64(iss);

    auto txn = db.CreateReadTxn();
    std::stringstream oss;

    auto fd = lgraph_api::FieldData::Int64(message_id);
    auto iit = txn.GetVertexIndexIterator(COMMENT, COMMENT_ID, fd, fd);
    if (iit.IsValid()) {
        auto comment = txn.GetVertexIterator(iit.GetVid());
        WriteInt64(oss, comment[COMMENT_CREATIONDATE].integer());
        WriteString(oss, comment[COMMENT_CONTENT].string());
    } else {
        auto post = txn.GetVertexByUniqueIndex(POST, POST_ID, fd);
        WriteInt64(oss, post[POST_CREATIONDATE].integer());
        auto content = post[POST_CONTENT];
        if (!content.is_null()) {
            WriteString(oss, content.string());
        } else {
            WriteString(oss, post[POST_IMAGEFILE].string());
        }
    }

    response = oss.str();
    return true;
}
