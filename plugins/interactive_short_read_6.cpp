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
    int64_t forum_vid;
    if (iit.IsValid()) {
        auto message = txn.GetVertexIterator(iit.GetVid());
        while (true) {
            auto reply_of_comment = message[COMMENT_REPLYOFCOMMENT];
            if (!reply_of_comment.is_null()) {
                message.Goto(reply_of_comment.integer());
            } else {
                message.Goto(message[COMMENT_REPLYOFPOST].integer());
                forum_vid = message[POST_CONTAINER].integer();
                break;
            }
        }
    } else {
        auto post = txn.GetVertexByUniqueIndex(POST, POST_ID, fd);
        forum_vid = post[POST_CONTAINER].integer();
    }
    auto forum = txn.GetVertexIterator(forum_vid);
    WriteInt64(oss, forum[FORUM_ID].integer());
    WriteString(oss, forum[FORUM_TITLE].string());
    auto moderator = txn.GetVertexIterator(forum[FORUM_MODERATOR].integer());
    WriteInt64(oss, moderator[PERSON_ID].integer());
    WriteString(oss, moderator[PERSON_FIRSTNAME].string());
    WriteString(oss, moderator[PERSON_LASTNAME].string());

    response = oss.str();
    return true;
}
