#!/bin/bash
# Comprehensive Business Logic Tests

BASE_URL="http://localhost:8080"
FAILED_TESTS=0
PASSED_TESTS=0

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

print_test() {
    echo -e "${BLUE}▶ $1${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
    ((PASSED_TESTS++))
}

print_fail() {
    echo -e "${RED}✗ $1${NC}"
    echo -e "${RED}  Response: $2${NC}"
    ((FAILED_TESTS++))
}

print_section() {
    echo ""
    echo -e "${YELLOW}═══════════════════════════════════════════════════════════${NC}"
    echo -e "${YELLOW}$1${NC}"
    echo -e "${YELLOW}═══════════════════════════════════════════════════════════${NC}"
}

# Helper function to check JSON field
check_json_field() {
    local response="$1"
    local field="$2"
    local expected="$3"
    
    if echo "$response" | grep -q "\"$field\""; then
        if [ -n "$expected" ]; then
            if echo "$response" | grep -q "\"$field\":\"$expected\"" || \
               echo "$response" | grep -q "\"$field\":$expected"; then
                return 0
            else
                return 1
            fi
        else
            return 0
        fi
    else
        return 1
    fi
}

print_section "🧪 BUSINESS LOGIC TESTS"

# ============================================================================
# TEST SUITE 1: AUTHENTICATION BUSINESS LOGIC
# ============================================================================
print_section "1️⃣  AUTHENTICATION BUSINESS LOGIC"

# Test 1.1: Login with valid credentials
print_test "Test 1.1: Login with valid credentials"
LOGIN_RESP=$(curl -s -X POST "$BASE_URL/api/v1/auth/login" \
    -H "Content-Type: application/json" \
    -d '{"username":"admin","password":"admin123"}')

if check_json_field "$LOGIN_RESP" "access_token" && \
   check_json_field "$LOGIN_RESP" "token_type" "Bearer"; then
    ADMIN_TOKEN=$(echo "$LOGIN_RESP" | grep -o '"access_token":"[^"]*"' | cut -d'"' -f4)
    print_success "Valid login returns JWT token"
else
    print_fail "Valid login should return JWT token" "$LOGIN_RESP"
fi

# Test 1.2: Login with invalid credentials
print_test "Test 1.2: Login with INVALID credentials"
INVALID_LOGIN=$(curl -s -X POST "$BASE_URL/api/v1/auth/login" \
    -H "Content-Type: application/json" \
    -d '{"username":"admin","password":"wrongpassword"}')

if check_json_field "$INVALID_LOGIN" "error" "Authentication failed"; then
    print_success "Invalid credentials rejected"
else
    print_fail "Should reject invalid credentials" "$INVALID_LOGIN"
fi

# Test 1.3: Login with non-existent user
print_test "Test 1.3: Login with non-existent user"
NONEXIST_LOGIN=$(curl -s -X POST "$BASE_URL/api/v1/auth/login" \
    -H "Content-Type: application/json" \
    -d '{"username":"nonexistentuser","password":"anypass"}')

if check_json_field "$NONEXIST_LOGIN" "error"; then
    print_success "Non-existent user rejected"
else
    print_fail "Should reject non-existent user" "$NONEXIST_LOGIN"
fi

# Test 1.4: Login with empty credentials
print_test "Test 1.4: Login with empty username"
EMPTY_LOGIN=$(curl -s -X POST "$BASE_URL/api/v1/auth/login" \
    -H "Content-Type: application/json" \
    -d '{"username":"","password":"admin123"}')

if check_json_field "$EMPTY_LOGIN" "error"; then
    print_success "Empty username rejected"
else
    print_fail "Should reject empty username" "$EMPTY_LOGIN"
fi

# Test 1.5: Register new user
print_test "Test 1.5: Register new user"
NEW_USERNAME="testuser_$(date +%s)"
REGISTER_RESP=$(curl -s -X POST "$BASE_URL/api/v1/auth/register" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$NEW_USERNAME\",\"password\":\"testpass123\"}")

if check_json_field "$REGISTER_RESP" "access_token" && \
   check_json_field "$REGISTER_RESP" "username" "$NEW_USERNAME"; then
    NEW_TOKEN=$(echo "$REGISTER_RESP" | grep -o '"access_token":"[^"]*"' | cut -d'"' -f4)
    print_success "User registration successful"
else
    print_fail "Registration should return token" "$REGISTER_RESP"
fi

# Test 1.6: Register duplicate username
print_test "Test 1.6: Register duplicate username"
DUP_REGISTER=$(curl -s -X POST "$BASE_URL/api/v1/auth/register" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$NEW_USERNAME\",\"password\":\"testpass123\"}")

if check_json_field "$DUP_REGISTER" "error"; then
    print_success "Duplicate username rejected"
else
    print_fail "Should reject duplicate username" "$DUP_REGISTER"
fi

# Test 1.7: Get user info with valid token
print_test "Test 1.7: Get user info with valid token"
USER_INFO=$(curl -s -X GET "$BASE_URL/api/v1/auth/me" \
    -H "Authorization: Bearer $ADMIN_TOKEN")

if check_json_field "$USER_INFO" "username" "admin" && \
   check_json_field "$USER_INFO" "role" "admin"; then
    print_success "Token returns correct user info"
else
    print_fail "Should return user info" "$USER_INFO"
fi

# Test 1.8: Get user info with invalid token
print_test "Test 1.8: Get user info with INVALID token"
INVALID_TOKEN_INFO=$(curl -s -X GET "$BASE_URL/api/v1/auth/me" \
    -H "Authorization: Bearer invalid_token_12345")

if check_json_field "$INVALID_TOKEN_INFO" "error"; then
    print_success "Invalid token rejected"
else
    print_fail "Should reject invalid token" "$INVALID_TOKEN_INFO"
fi

# Test 1.9: Access protected endpoint without token
print_test "Test 1.9: Access protected endpoint without token"
NO_TOKEN=$(curl -s -X GET "$BASE_URL/api/v1/files")

if check_json_field "$NO_TOKEN" "error" "Authentication required"; then
    print_success "Protected endpoint requires authentication"
else
    print_fail "Should require authentication" "$NO_TOKEN"
fi

# ============================================================================
# TEST SUITE 2: FILE OPERATIONS BUSINESS LOGIC
# ============================================================================
print_section "2️⃣  FILE OPERATIONS BUSINESS LOGIC"

# Test 2.1: Upload file with authentication
print_test "Test 2.1: Upload file with valid token"
echo "Test file content $(date)" > /tmp/test_business.txt
UPLOAD_RESP=$(curl -s -X POST "$BASE_URL/api/v1/files/upload" \
    -H "Authorization: Bearer $ADMIN_TOKEN" \
    -F "file=@/tmp/test_business.txt")

if check_json_field "$UPLOAD_RESP" "success" "true" && \
   check_json_field "$UPLOAD_RESP" "filename"; then
    UPLOADED_FILE=$(echo "$UPLOAD_RESP" | grep -o '"filename":"[^"]*"' | cut -d'"' -f4)
    print_success "File upload successful"
else
    print_fail "File upload should succeed" "$UPLOAD_RESP"
fi

# Test 2.2: Upload file without authentication
print_test "Test 2.2: Upload file WITHOUT token"
UPLOAD_NO_AUTH=$(curl -s -X POST "$BASE_URL/api/v1/files/upload" \
    -F "file=@/tmp/test_business.txt")

if check_json_field "$UPLOAD_NO_AUTH" "error" "Authentication required"; then
    print_success "Upload requires authentication"
else
    print_fail "Should require authentication" "$UPLOAD_NO_AUTH"
fi

# Test 2.3: Upload without file
print_test "Test 2.3: Upload without file data"
UPLOAD_NO_FILE=$(curl -s -X POST "$BASE_URL/api/v1/files/upload" \
    -H "Authorization: Bearer $ADMIN_TOKEN")

if check_json_field "$UPLOAD_NO_FILE" "error" "No file provided"; then
    print_success "Empty upload rejected"
else
    print_fail "Should reject empty upload" "$UPLOAD_NO_FILE"
fi

# Test 2.4: Download existing file
if [ -n "$UPLOADED_FILE" ]; then
    print_test "Test 2.4: Download existing file"
    DOWNLOAD_RESP=$(curl -s -X GET "$BASE_URL/api/v1/files/download/$UPLOADED_FILE" \
        -H "Authorization: Bearer $ADMIN_TOKEN")
    
    if echo "$DOWNLOAD_RESP" | grep -q "Test file content"; then
        print_success "File download successful"
    else
        print_fail "Should download file content" "$DOWNLOAD_RESP"
    fi
fi

# Test 2.5: Download non-existent file
print_test "Test 2.5: Download NON-EXISTENT file"
DOWNLOAD_404=$(curl -s -X GET "$BASE_URL/api/v1/files/download/nonexistent_file.txt" \
    -H "Authorization: Bearer $ADMIN_TOKEN")

if check_json_field "$DOWNLOAD_404" "success" "false" || \
   echo "$DOWNLOAD_404" | grep -q "not found" || \
   echo "$DOWNLOAD_404" | grep -q "error"; then
    print_success "Non-existent file returns error"
else
    print_fail "Should return error for non-existent file" "$DOWNLOAD_404"
fi

# Test 2.6: List files
print_test "Test 2.6: List files"
LIST_RESP=$(curl -s -X GET "$BASE_URL/api/v1/files" \
    -H "Authorization: Bearer $ADMIN_TOKEN")

if check_json_field "$LIST_RESP" "success" "true" && \
   check_json_field "$LIST_RESP" "files"; then
    print_success "File listing successful"
else
    print_fail "Should list files" "$LIST_RESP"
fi

# Test 2.7: Delete existing file
if [ -n "$UPLOADED_FILE" ]; then
    print_test "Test 2.7: Delete existing file"
    DELETE_RESP=$(curl -s -X DELETE "$BASE_URL/api/v1/files/$UPLOADED_FILE" \
        -H "Authorization: Bearer $ADMIN_TOKEN")
    
    if check_json_field "$DELETE_RESP" "success" "true"; then
        print_success "File deletion successful"
    else
        print_fail "Should delete file" "$DELETE_RESP"
    fi
fi

# Test 2.8: Delete non-existent file
print_test "Test 2.8: Delete NON-EXISTENT file"
DELETE_404=$(curl -s -X DELETE "$BASE_URL/api/v1/files/nonexistent_file.txt" \
    -H "Authorization: Bearer $ADMIN_TOKEN")

if check_json_field "$DELETE_404" "success" "false" || \
   check_json_field "$DELETE_404" "error"; then
    print_success "Non-existent file deletion returns error"
else
    print_fail "Should return error for non-existent file" "$DELETE_404"
fi

# ============================================================================
# TEST SUITE 3: SESSION MANAGEMENT BUSINESS LOGIC
# ============================================================================
print_section "3️⃣  SESSION MANAGEMENT BUSINESS LOGIC"

# Test 3.1: Logout with valid token
print_test "Test 3.1: Logout with valid token"
LOGOUT_RESP=$(curl -s -X POST "$BASE_URL/api/v1/auth/logout" \
    -H "Authorization: Bearer $NEW_TOKEN")

if check_json_field "$LOGOUT_RESP" "message" "Logged out successfully" || \
   check_json_field "$LOGOUT_RESP" "success"; then
    print_success "Logout successful"
else
    print_fail "Logout should succeed" "$LOGOUT_RESP"
fi

# Test 3.2: Use token after logout
print_test "Test 3.2: Use token AFTER logout"
AFTER_LOGOUT=$(curl -s -X GET "$BASE_URL/api/v1/auth/me" \
    -H "Authorization: Bearer $NEW_TOKEN")

if check_json_field "$AFTER_LOGOUT" "error"; then
    print_success "Token invalid after logout"
else
    print_fail "Token should be invalid after logout" "$AFTER_LOGOUT"
fi

# Test 3.3: Multiple simultaneous logins (same user)
print_test "Test 3.3: Multiple logins for same user"
LOGIN1=$(curl -s -X POST "$BASE_URL/api/v1/auth/login" \
    -H "Content-Type: application/json" \
    -d '{"username":"user","password":"user123"}')
TOKEN1=$(echo "$LOGIN1" | grep -o '"access_token":"[^"]*"' | cut -d'"' -f4)

LOGIN2=$(curl -s -X POST "$BASE_URL/api/v1/auth/login" \
    -H "Content-Type: application/json" \
    -d '{"username":"user","password":"user123"}')
TOKEN2=$(echo "$LOGIN2" | grep -o '"access_token":"[^"]*"' | cut -d'"' -f4)

if [ -n "$TOKEN1" ] && [ -n "$TOKEN2" ] && [ "$TOKEN1" != "$TOKEN2" ]; then
    print_success "Multiple sessions supported"
else
    print_fail "Should support multiple sessions" "TOKEN1=$TOKEN1, TOKEN2=$TOKEN2"
fi

# ============================================================================
# TEST SUITE 4: EDGE CASES & BOUNDARY CONDITIONS
# ============================================================================
print_section "4️⃣  EDGE CASES & BOUNDARY CONDITIONS"

# Test 4.1: Very long username
print_test "Test 4.1: Username length validation"
LONG_USERNAME="verylongusername$(printf 'a%.0s' {1..100})"
LONG_USER_REG=$(curl -s -X POST "$BASE_URL/api/v1/auth/register" \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$LONG_USERNAME\",\"password\":\"pass123\"}")

# Should either accept or reject gracefully
if check_json_field "$LONG_USER_REG" "error" || \
   check_json_field "$LONG_USER_REG" "access_token"; then
    print_success "Long username handled correctly"
else
    print_fail "Should handle long username" "$LONG_USER_REG"
fi

# Test 4.2: Short password
print_test "Test 4.2: Short password validation"
SHORT_PASS_REG=$(curl -s -X POST "$BASE_URL/api/v1/auth/register" \
    -H "Content-Type: application/json" \
    -d '{"username":"shortpass_user","password":"123"}')

# Should either accept or reject gracefully
if check_json_field "$SHORT_PASS_REG" "error" || \
   check_json_field "$SHORT_PASS_REG" "access_token"; then
    print_success "Short password handled"
else
    print_fail "Should handle short password" "$SHORT_PASS_REG"
fi

# Test 4.3: Special characters in username
print_test "Test 4.3: Special characters in username"
SPECIAL_USER=$(curl -s -X POST "$BASE_URL/api/v1/auth/register" \
    -H "Content-Type: application/json" \
    -d '{"username":"user@#$%","password":"testpass123"}')

if check_json_field "$SPECIAL_USER" "error" || \
   check_json_field "$SPECIAL_USER" "access_token"; then
    print_success "Special characters handled"
else
    print_fail "Should handle special characters" "$SPECIAL_USER"
fi

# Test 4.4: SQL injection attempt
print_test "Test 4.4: SQL injection protection"
SQL_INJECT=$(curl -s -X POST "$BASE_URL/api/v1/auth/login" \
    -H "Content-Type: application/json" \
    -d '{"username":"admin","password":"' OR '1'='1"}')

if check_json_field "$SQL_INJECT" "error"; then
    print_success "SQL injection blocked"
else
    print_fail "Should block SQL injection" "$SQL_INJECT"
fi

# Test 4.5: Missing Content-Type header
print_test "Test 4.5: Missing Content-Type header"
NO_CONTENT_TYPE=$(curl -s -X POST "$BASE_URL/api/v1/auth/login" \
    -d '{"username":"admin","password":"admin123"}')

# Should handle gracefully
if check_json_field "$NO_CONTENT_TYPE" "access_token" || \
   check_json_field "$NO_CONTENT_TYPE" "error"; then
    print_success "Missing Content-Type handled"
else
    print_fail "Should handle missing Content-Type" "$NO_CONTENT_TYPE"
fi

# ============================================================================
# TEST SUITE 5: CORS & HEADERS
# ============================================================================
print_section "5️⃣  CORS & SECURITY HEADERS"

# Test 5.1: CORS headers present
print_test "Test 5.1: CORS headers present"
CORS_RESP=$(curl -s -i -X OPTIONS "$BASE_URL/api/v1/auth/login" 2>&1)

if echo "$CORS_RESP" | grep -qi "Access-Control-Allow-Origin"; then
    print_success "CORS headers present"
else
    print_fail "Should include CORS headers" "$CORS_RESP"
fi

# Test 5.2: OPTIONS request handling
print_test "Test 5.2: OPTIONS preflight request"
OPTIONS_RESP=$(curl -s -o /dev/null -w "%{http_code}" -X OPTIONS "$BASE_URL/api/v1/auth/login")

if [ "$OPTIONS_RESP" = "200" ]; then
    print_success "OPTIONS request handled"
else
    print_fail "Should handle OPTIONS request" "HTTP $OPTIONS_RESP"
fi

# ============================================================================
# TEST SUITE 6: SYSTEM HEALTH
# ============================================================================
print_section "6️⃣  SYSTEM HEALTH & MONITORING"

# Test 6.1: Health endpoint
print_test "Test 6.1: Health endpoint"
HEALTH=$(curl -s -X GET "$BASE_URL/health")

if check_json_field "$HEALTH" "status" "healthy"; then
    print_success "Health endpoint functional"
else
    print_fail "Health should return healthy" "$HEALTH"
fi

# Test 6.2: Invalid endpoint (404)
print_test "Test 6.2: Invalid endpoint returns 404"
NOT_FOUND=$(curl -s -X GET "$BASE_URL/api/v1/invalid/endpoint" \
    -H "Authorization: Bearer $ADMIN_TOKEN")

if check_json_field "$NOT_FOUND" "error"; then
    print_success "404 for invalid endpoints"
else
    print_fail "Should return 404" "$NOT_FOUND"
fi

# ============================================================================
# SUMMARY
# ============================================================================
print_section "📊 TEST SUMMARY"

TOTAL_TESTS=$((PASSED_TESTS + FAILED_TESTS))
SUCCESS_RATE=$((PASSED_TESTS * 100 / TOTAL_TESTS))

echo ""
echo -e "${GREEN}✓ Passed: $PASSED_TESTS${NC}"
echo -e "${RED}✗ Failed: $FAILED_TESTS${NC}"
echo -e "${BLUE}Total:   $TOTAL_TESTS${NC}"
echo ""
echo -e "${BLUE}Success Rate: ${SUCCESS_RATE}%${NC}"
echo ""

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}╔═══════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║           ✅ ALL TESTS PASSED! ✅                         ║${NC}"
    echo -e "${GREEN}╚═══════════════════════════════════════════════════════════╝${NC}"
    exit 0
else
    echo -e "${RED}╔═══════════════════════════════════════════════════════════╗${NC}"
    echo -e "${RED}║           ⚠️  SOME TESTS FAILED ⚠️                        ║${NC}"
    echo -e "${RED}╚═══════════════════════════════════════════════════════════╝${NC}"
    exit 1
fi

