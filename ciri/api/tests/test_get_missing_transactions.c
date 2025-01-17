/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Refer to the LICENSE file for licensing information
 */

#include <unity/unity.h>

#include "ciri/api/api.h"
#include "ciri/consensus/test_utils/tangle.h"
#include "ciri/node/node.h"

static char *test_db_path = "ciri/api/tests/tangle-test.db";
static char *ciri_db_path = "ciri/api/tests/tangle.db";
static iota_api_t api;
static connection_config_t config;
static tangle_t tangle;
static core_t core;

void test_get_missing_transactions_empty(void) {
  get_missing_transactions_res_t *res = get_missing_transactions_res_new();
  error_res_t *error = NULL;

  TEST_ASSERT_EQUAL_INT(get_missing_transactions_res_hash_num(res), 0);
  TEST_ASSERT(iota_api_get_missing_transactions(&api, res, &error) == RC_OK);
  TEST_ASSERT(error == NULL);
  TEST_ASSERT_EQUAL_INT(get_missing_transactions_res_hash_num(res), 0);

  get_missing_transactions_res_free(&res);
  error_res_free(&error);
}

void test_get_tips(void) {
  get_missing_transactions_res_t *res = get_missing_transactions_res_new();
  error_res_t *error = NULL;
  flex_trit_t hashes[10][FLEX_TRIT_SIZE_243];
  tryte_t trytes[81] =
      "A99999999999999999999999999999999999999999999999999999999999999999999999"
      "999999999";

  for (size_t i = 0; i < 10; i++) {
    flex_trits_from_trytes(hashes[i], HASH_LENGTH_TRIT, trytes, HASH_LENGTH_TRYTE, HASH_LENGTH_TRYTE);
    trytes[0]++;
  }

  TEST_ASSERT(request_transaction(&api.core->node.transaction_requester, &tangle, hashes[0], true) == RC_OK);
  TEST_ASSERT(request_transaction(&api.core->node.transaction_requester, &tangle, hashes[1], false) == RC_OK);
  TEST_ASSERT(request_transaction(&api.core->node.transaction_requester, &tangle, hashes[2], true) == RC_OK);
  TEST_ASSERT(request_transaction(&api.core->node.transaction_requester, &tangle, hashes[3], false) == RC_OK);
  TEST_ASSERT(request_transaction(&api.core->node.transaction_requester, &tangle, hashes[4], true) == RC_OK);
  TEST_ASSERT(request_transaction(&api.core->node.transaction_requester, &tangle, hashes[5], false) == RC_OK);
  TEST_ASSERT(request_transaction(&api.core->node.transaction_requester, &tangle, hashes[6], true) == RC_OK);
  TEST_ASSERT(request_transaction(&api.core->node.transaction_requester, &tangle, hashes[7], false) == RC_OK);
  TEST_ASSERT(request_transaction(&api.core->node.transaction_requester, &tangle, hashes[8], true) == RC_OK);
  TEST_ASSERT(request_transaction(&api.core->node.transaction_requester, &tangle, hashes[9], false) == RC_OK);

  TEST_ASSERT(iota_api_get_missing_transactions(&api, res, &error) == RC_OK);
  TEST_ASSERT(error == NULL);
  TEST_ASSERT_EQUAL_INT(get_missing_transactions_res_hash_num(res), 10);

  {
    hash243_stack_entry_t *iter = res->hashes;

    TEST_ASSERT_EQUAL_MEMORY(iter->hash, hashes[8], FLEX_TRIT_SIZE_243);
    iter = iter->next;
    TEST_ASSERT_EQUAL_MEMORY(iter->hash, hashes[6], FLEX_TRIT_SIZE_243);
    iter = iter->next;
    TEST_ASSERT_EQUAL_MEMORY(iter->hash, hashes[4], FLEX_TRIT_SIZE_243);
    iter = iter->next;
    TEST_ASSERT_EQUAL_MEMORY(iter->hash, hashes[2], FLEX_TRIT_SIZE_243);
    iter = iter->next;
    TEST_ASSERT_EQUAL_MEMORY(iter->hash, hashes[0], FLEX_TRIT_SIZE_243);
    iter = iter->next;
    TEST_ASSERT_EQUAL_MEMORY(iter->hash, hashes[9], FLEX_TRIT_SIZE_243);
    iter = iter->next;
    TEST_ASSERT_EQUAL_MEMORY(iter->hash, hashes[7], FLEX_TRIT_SIZE_243);
    iter = iter->next;
    TEST_ASSERT_EQUAL_MEMORY(iter->hash, hashes[5], FLEX_TRIT_SIZE_243);
    iter = iter->next;
    TEST_ASSERT_EQUAL_MEMORY(iter->hash, hashes[3], FLEX_TRIT_SIZE_243);
    iter = iter->next;
    TEST_ASSERT_EQUAL_MEMORY(iter->hash, hashes[1], FLEX_TRIT_SIZE_243);
    iter = iter->next;
  }

  get_missing_transactions_res_free(&res);
  error_res_free(&error);
}

int main(void) {
  UNITY_BEGIN();
  TEST_ASSERT(storage_init() == RC_OK);

  api.core = &core;
  config.db_path = test_db_path;
  TEST_ASSERT(tangle_setup(&tangle, &config, test_db_path, ciri_db_path) == RC_OK);
  TEST_ASSERT(iota_node_conf_init(&api.core->node.conf) == RC_OK);
  TEST_ASSERT(requester_init(&api.core->node.transaction_requester, &api.core->node) == RC_OK);

  RUN_TEST(test_get_missing_transactions_empty);
  RUN_TEST(test_get_tips);

  TEST_ASSERT(requester_destroy(&api.core->node.transaction_requester) == RC_OK);
  TEST_ASSERT(tangle_cleanup(&tangle, test_db_path) == RC_OK);

  TEST_ASSERT(storage_destroy() == RC_OK);
  return UNITY_END();
}
