# OpenAuto REST API Testing Guide

This comprehensive guide covers testing strategies, validation procedures, and automated testing approaches for the OpenAuto REST API server.

## Table of Contents

1. [Quick Validation](#quick-validation)
2. [Manual Testing](#manual-testing)
3. [Automated Testing](#automated-testing)
4. [Performance Testing](#performance-testing)
5. [Security Testing](#security-testing)
6. [Integration Testing](#integration-testing)
7. [Load Testing](#load-testing)
8. [API Documentation Testing](#api-documentation-testing)

## Quick Validation

### Server Health Check

```bash
# Basic server health
curl -v http://localhost:8080/api/health

# Expected response:
# HTTP/1.1 200 OK
# Content-Type: application/json
# {
#   "status": "healthy",
#   "timestamp": "2024-01-01T12:00:00Z",
#   "uptime": 3600,
#   "version": "1.0.0"
# }
```

### OpenAPI Documentation

```bash
# Swagger UI (should return HTML)
curl -v http://localhost:8080/docs

# OpenAPI spec (should return JSON)
curl -v http://localhost:8080/openapi.json

# ReDoc documentation (should return HTML)
curl -v http://localhost:8080/redoc
```

### Core Endpoints

```bash
# Configuration
curl -v http://localhost:8080/api/config

# State
curl -v http://localhost:8080/api/state

# Event status
curl -v http://localhost:8080/api/events/status

# Logs
curl -v "http://localhost:8080/api/logs?limit=5"
```

## Manual Testing

### Testing Script

Create a comprehensive testing script:

```bash
#!/bin/bash
# File: test_rest_api.sh

API_BASE="http://localhost:8080"
LOG_FILE="api_test_results.log"

echo "OpenAuto REST API Test Suite" | tee $LOG_FILE
echo "============================" | tee -a $LOG_FILE
echo "Test started: $(date)" | tee -a $LOG_FILE
echo "" | tee -a $LOG_FILE

# Test counter
TESTS=0
PASSED=0
FAILED=0

test_endpoint() {
    local name="$1"
    local url="$2"
    local expected_code="$3"
    local method="${4:-GET}"
    local data="$5"
    
    TESTS=$((TESTS + 1))
    echo "Test $TESTS: $name" | tee -a $LOG_FILE
    
    if [ -n "$data" ]; then
        response=$(curl -s -w "%{http_code}" -X "$method" \
            -H "Content-Type: application/json" \
            -d "$data" "$url")
    else
        response=$(curl -s -w "%{http_code}" -X "$method" "$url")
    fi
    
    http_code="${response: -3}"
    body="${response%???}"
    
    if [ "$http_code" = "$expected_code" ]; then
        echo "  ✅ PASS (HTTP $http_code)" | tee -a $LOG_FILE
        PASSED=$((PASSED + 1))
        if [ -n "$body" ] && command -v jq >/dev/null; then
            echo "$body" | jq . >/dev/null 2>&1 && echo "  📄 Valid JSON" | tee -a $LOG_FILE
        fi
    else
        echo "  ❌ FAIL (Expected: $expected_code, Got: $http_code)" | tee -a $LOG_FILE
        echo "  Response: $body" | tee -a $LOG_FILE
        FAILED=$((FAILED + 1))
    fi
    echo "" | tee -a $LOG_FILE
}

# Health and documentation tests
test_endpoint "Health Check" "$API_BASE/api/health" "200"
test_endpoint "OpenAPI Specification" "$API_BASE/openapi.json" "200"
test_endpoint "Swagger UI" "$API_BASE/docs" "200"
test_endpoint "ReDoc Documentation" "$API_BASE/redoc" "200"

# Core API tests
test_endpoint "Configuration Get" "$API_BASE/api/config" "200"
test_endpoint "State Get" "$API_BASE/api/state" "200"
test_endpoint "Event Status" "$API_BASE/api/events/status" "200"
test_endpoint "Logs Get" "$API_BASE/api/logs?limit=5" "200"

# POST/PUT tests
test_endpoint "Configuration Update" "$API_BASE/api/config/ui.brightness" "200" "PUT" '{"value": 75}'
test_endpoint "Event Post" "$API_BASE/api/events" "201" "POST" '{"type": "test", "data": {"message": "test"}}'

# Error tests
test_endpoint "Not Found" "$API_BASE/api/nonexistent" "404"
test_endpoint "Invalid JSON" "$API_BASE/api/config/test" "400" "PUT" '{"invalid": json}'

# Authentication tests (if enabled)
test_endpoint "Protected Endpoint (No Auth)" "$API_BASE/api/admin/status" "401"
test_endpoint "Protected Endpoint (Invalid Auth)" "$API_BASE/api/admin/status" "401" "GET" "" "Authorization: Bearer invalid_token"

echo "Test Summary:" | tee -a $LOG_FILE
echo "=============" | tee -a $LOG_FILE
echo "Total Tests: $TESTS" | tee -a $LOG_FILE
echo "Passed: $PASSED" | tee -a $LOG_FILE
echo "Failed: $FAILED" | tee -a $LOG_FILE
echo "Success Rate: $(( (PASSED * 100) / TESTS ))%" | tee -a $LOG_FILE
echo "Test completed: $(date)" | tee -a $LOG_FILE

if [ $FAILED -eq 0 ]; then
    echo "🎉 All tests passed!" | tee -a $LOG_FILE
    exit 0
else
    echo "⚠️  Some tests failed. Check the log for details." | tee -a $LOG_FILE
    exit 1
fi
```

Make the script executable and run it:

```bash
chmod +x test_rest_api.sh
./test_rest_api.sh
```

### Interactive Testing with HTTPie

If you prefer HTTPie over curl:

```bash
# Install HTTPie
pip install httpie

# Basic tests
http GET localhost:8080/api/health
http GET localhost:8080/api/config
http PUT localhost:8080/api/config/ui.brightness value:=75
http POST localhost:8080/api/events type=test data:='{"message": "test"}'
```

## Automated Testing

### Python Test Suite

Create a Python-based test suite:

```python
#!/usr/bin/env python3
# File: test_api.py

import requests
import json
import sys
from datetime import datetime

class ApiTester:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
        self.session = requests.Session()
        self.tests_run = 0
        self.tests_passed = 0
        
    def test_get(self, endpoint, expected_status=200, description=""):
        """Test GET endpoint"""
        self.tests_run += 1
        url = f"{self.base_url}{endpoint}"
        
        try:
            response = self.session.get(url, timeout=10)
            if response.status_code == expected_status:
                print(f"✅ GET {endpoint} - {description}")
                self.tests_passed += 1
                return response
            else:
                print(f"❌ GET {endpoint} - Expected {expected_status}, got {response.status_code}")
                return None
        except Exception as e:
            print(f"❌ GET {endpoint} - Exception: {e}")
            return None
    
    def test_post(self, endpoint, data, expected_status=201, description=""):
        """Test POST endpoint"""
        self.tests_run += 1
        url = f"{self.base_url}{endpoint}"
        
        try:
            response = self.session.post(url, json=data, timeout=10)
            if response.status_code == expected_status:
                print(f"✅ POST {endpoint} - {description}")
                self.tests_passed += 1
                return response
            else:
                print(f"❌ POST {endpoint} - Expected {expected_status}, got {response.status_code}")
                return None
        except Exception as e:
            print(f"❌ POST {endpoint} - Exception: {e}")
            return None
    
    def test_put(self, endpoint, data, expected_status=200, description=""):
        """Test PUT endpoint"""
        self.tests_run += 1
        url = f"{self.base_url}{endpoint}"
        
        try:
            response = self.session.put(url, json=data, timeout=10)
            if response.status_code == expected_status:
                print(f"✅ PUT {endpoint} - {description}")
                self.tests_passed += 1
                return response
            else:
                print(f"❌ PUT {endpoint} - Expected {expected_status}, got {response.status_code}")
                return None
        except Exception as e:
            print(f"❌ PUT {endpoint} - Exception: {e}")
            return None
    
    def run_all_tests(self):
        """Run complete test suite"""
        print("OpenAuto REST API Test Suite")
        print("============================")
        print(f"Started: {datetime.now()}")
        print()
        
        # Health and documentation
        self.test_get("/api/health", description="Health check")
        self.test_get("/openapi.json", description="OpenAPI spec")
        self.test_get("/docs", description="Swagger UI")
        self.test_get("/redoc", description="ReDoc")
        
        # Core API endpoints
        self.test_get("/api/config", description="Configuration")
        self.test_get("/api/state", description="State")
        self.test_get("/api/events/status", description="Event status")
        self.test_get("/api/logs?limit=5", description="Logs")
        
        # Write operations
        self.test_put("/api/config/ui.brightness", {"value": 75}, description="Config update")
        self.test_post("/api/events", {"type": "test", "data": {"message": "test"}}, description="Event creation")
        
        # Error cases
        self.test_get("/api/nonexistent", expected_status=404, description="Not found")
        
        # Summary
        print()
        print("Test Summary:")
        print("=============")
        print(f"Total Tests: {self.tests_run}")
        print(f"Passed: {self.tests_passed}")
        print(f"Failed: {self.tests_run - self.tests_passed}")
        print(f"Success Rate: {(self.tests_passed / self.tests_run * 100):.1f}%")
        
        return self.tests_passed == self.tests_run

if __name__ == "__main__":
    tester = ApiTester()
    success = tester.run_all_tests()
    sys.exit(0 if success else 1)
```

Run the Python test suite:

```bash
python3 test_api.py
```

### JavaScript/Node.js Test Suite

```javascript
// File: test-api.js
const axios = require('axios');

class ApiTester {
    constructor(baseUrl = 'http://localhost:8080') {
        this.baseUrl = baseUrl;
        this.testsRun = 0;
        this.testsPassed = 0;
    }
    
    async testGet(endpoint, expectedStatus = 200, description = '') {
        this.testsRun++;
        try {
            const response = await axios.get(`${this.baseUrl}${endpoint}`);
            if (response.status === expectedStatus) {
                console.log(`✅ GET ${endpoint} - ${description}`);
                this.testsPassed++;
                return response;
            }
        } catch (error) {
            if (error.response && error.response.status === expectedStatus) {
                console.log(`✅ GET ${endpoint} - ${description}`);
                this.testsPassed++;
                return error.response;
            }
            console.log(`❌ GET ${endpoint} - ${error.message}`);
        }
        return null;
    }
    
    async testPost(endpoint, data, expectedStatus = 201, description = '') {
        this.testsRun++;
        try {
            const response = await axios.post(`${this.baseUrl}${endpoint}`, data);
            if (response.status === expectedStatus) {
                console.log(`✅ POST ${endpoint} - ${description}`);
                this.testsPassed++;
                return response;
            }
        } catch (error) {
            if (error.response && error.response.status === expectedStatus) {
                console.log(`✅ POST ${endpoint} - ${description}`);
                this.testsPassed++;
                return error.response;
            }
            console.log(`❌ POST ${endpoint} - ${error.message}`);
        }
        return null;
    }
    
    async runAllTests() {
        console.log('OpenAuto REST API Test Suite');
        console.log('============================');
        console.log(`Started: ${new Date()}`);
        console.log();
        
        // Health and documentation
        await this.testGet('/api/health', 200, 'Health check');
        await this.testGet('/openapi.json', 200, 'OpenAPI spec');
        await this.testGet('/docs', 200, 'Swagger UI');
        await this.testGet('/redoc', 200, 'ReDoc');
        
        // Core API endpoints
        await this.testGet('/api/config', 200, 'Configuration');
        await this.testGet('/api/state', 200, 'State');
        await this.testGet('/api/events/status', 200, 'Event status');
        await this.testGet('/api/logs?limit=5', 200, 'Logs');
        
        // Write operations
        await this.testPost('/api/events', {type: 'test', data: {message: 'test'}}, 201, 'Event creation');
        
        // Error cases
        await this.testGet('/api/nonexistent', 404, 'Not found');
        
        // Summary
        console.log();
        console.log('Test Summary:');
        console.log('=============');
        console.log(`Total Tests: ${this.testsRun}`);
        console.log(`Passed: ${this.testsPassed}`);
        console.log(`Failed: ${this.testsRun - this.testsPassed}`);
        console.log(`Success Rate: ${(this.testsPassed / this.testsRun * 100).toFixed(1)}%`);
        
        return this.testsPassed === this.testsRun;
    }
}

async function main() {
    const tester = new ApiTester();
    const success = await tester.runAllTests();
    process.exit(success ? 0 : 1);
}

main().catch(console.error);
```

## Performance Testing

### Simple Load Test

```bash
#!/bin/bash
# File: load_test.sh

API_BASE="http://localhost:8080"
CONCURRENT_USERS=10
REQUESTS_PER_USER=100
ENDPOINT="/api/health"

echo "Load Testing OpenAuto REST API"
echo "==============================="
echo "Endpoint: $ENDPOINT"
echo "Concurrent Users: $CONCURRENT_USERS"
echo "Requests per User: $REQUESTS_PER_USER"
echo "Total Requests: $((CONCURRENT_USERS * REQUESTS_PER_USER))"
echo ""

# Install Apache Bench if not available
if ! command -v ab >/dev/null; then
    echo "Installing Apache Bench..."
    sudo apt-get update && sudo apt-get install -y apache2-utils
fi

# Run load test
echo "Starting load test..."
ab -n $((CONCURRENT_USERS * REQUESTS_PER_USER)) \
   -c $CONCURRENT_USERS \
   -g results.tsv \
   "$API_BASE$ENDPOINT"

# Analyze results
echo ""
echo "Performance Summary:"
echo "==================="
grep "Requests per second" results.txt 2>/dev/null || echo "See above for detailed results"
grep "Time per request" results.txt 2>/dev/null || echo ""
```

### Advanced Performance Testing with wrk

```bash
# Install wrk
git clone https://github.com/wg/wrk.git
cd wrk && make && sudo cp wrk /usr/local/bin/

# Basic load test
wrk -t4 -c100 -d30s http://localhost:8080/api/health

# Custom script for complex scenarios
cat > test_script.lua << 'EOF'
request = function()
    paths = {"/api/health", "/api/config", "/api/state", "/api/events/status"}
    path = paths[math.random(#paths)]
    return wrk.format("GET", path)
end
EOF

wrk -t4 -c100 -d30s -s test_script.lua http://localhost:8080
```

## Security Testing

### Authentication Tests

```bash
# Test without authentication (should fail)
curl -v http://localhost:8080/api/admin/status

# Test with invalid token
curl -v -H "Authorization: Bearer invalid_token" http://localhost:8080/api/admin/status

# Test with malformed authorization header
curl -v -H "Authorization: invalid_format" http://localhost:8080/api/admin/status

# Test token injection attempts
curl -v -H "Authorization: Bearer ; rm -rf /" http://localhost:8080/api/admin/status
```

### Input Validation Tests

```bash
# SQL injection attempts
curl -X POST http://localhost:8080/api/events \
  -H "Content-Type: application/json" \
  -d '{"type": "test; DROP TABLE users;", "data": {}}'

# XSS attempts
curl -X POST http://localhost:8080/api/events \
  -H "Content-Type: application/json" \
  -d '{"type": "<script>alert(\"xss\")</script>", "data": {}}'

# Oversized payloads
curl -X POST http://localhost:8080/api/events \
  -H "Content-Type: application/json" \
  -d "$(python3 -c 'print("{\"data\": \"" + "A" * 1000000 + "\"}")')"

# Invalid JSON
curl -X POST http://localhost:8080/api/events \
  -H "Content-Type: application/json" \
  -d '{"invalid": json, syntax}'
```

## Integration Testing

### Event Bus Integration

```bash
# Test event publishing and consumption
curl -X POST http://localhost:8080/api/events \
  -H "Content-Type: application/json" \
  -d '{"type": "ui.brightness.changed", "data": {"brightness": 75}}'

# Verify event was processed
sleep 1
curl http://localhost:8080/api/events/status

# Check if configuration was updated
curl http://localhost:8080/api/config | jq '.ui.brightness'
```

### State Machine Integration

```bash
# Test state transitions
curl -X POST http://localhost:8080/api/state/transition \
  -H "Content-Type: application/json" \
  -d '{"state": "connected"}'

# Verify state change
curl http://localhost:8080/api/state | jq '.current_state'
```

### Configuration Integration

```bash
# Update configuration
curl -X PUT http://localhost:8080/api/config/audio.volume \
  -H "Content-Type: application/json" \
  -d '{"value": 85}'

# Verify configuration persistence
curl http://localhost:8080/api/config | jq '.audio.volume'

# Restart API server (if possible) and verify persistence
# curl http://localhost:8080/api/config | jq '.audio.volume'
```

## Load Testing

### Comprehensive Load Test

```python
#!/usr/bin/env python3
# File: comprehensive_load_test.py

import asyncio
import aiohttp
import time
import statistics
from concurrent.futures import ThreadPoolExecutor
import json

class LoadTester:
    def __init__(self, base_url="http://localhost:8080"):
        self.base_url = base_url
        self.results = []
        
    async def make_request(self, session, endpoint, method="GET", data=None):
        """Make a single request and measure response time"""
        start_time = time.time()
        try:
            if method == "GET":
                async with session.get(f"{self.base_url}{endpoint}") as response:
                    await response.text()
                    return time.time() - start_time, response.status
            elif method == "POST":
                async with session.post(f"{self.base_url}{endpoint}", json=data) as response:
                    await response.text()
                    return time.time() - start_time, response.status
        except Exception as e:
            return time.time() - start_time, 0
    
    async def worker(self, session, endpoints, requests_per_worker):
        """Worker coroutine that makes multiple requests"""
        worker_results = []
        for _ in range(requests_per_worker):
            endpoint = endpoints[len(worker_results) % len(endpoints)]
            duration, status = await self.make_request(session, endpoint)
            worker_results.append((duration, status))
            
        return worker_results
    
    async def run_load_test(self, concurrent_workers=10, requests_per_worker=100):
        """Run load test with specified parameters"""
        endpoints = [
            "/api/health",
            "/api/config",
            "/api/state",
            "/api/events/status",
            "/api/logs?limit=5"
        ]
        
        print(f"Load Test Configuration:")
        print(f"  Concurrent Workers: {concurrent_workers}")
        print(f"  Requests per Worker: {requests_per_worker}")
        print(f"  Total Requests: {concurrent_workers * requests_per_worker}")
        print(f"  Endpoints: {', '.join(endpoints)}")
        print()
        
        async with aiohttp.ClientSession() as session:
            start_time = time.time()
            
            # Create worker tasks
            tasks = []
            for _ in range(concurrent_workers):
                task = asyncio.create_task(
                    self.worker(session, endpoints, requests_per_worker)
                )
                tasks.append(task)
            
            # Wait for all workers to complete
            worker_results = await asyncio.gather(*tasks)
            
            total_time = time.time() - start_time
            
            # Flatten results
            all_results = []
            for results in worker_results:
                all_results.extend(results)
            
            self.analyze_results(all_results, total_time)
    
    def analyze_results(self, results, total_time):
        """Analyze and display test results"""
        durations = [r[0] for r in results]
        statuses = [r[1] for r in results]
        
        successful_requests = sum(1 for s in statuses if 200 <= s < 300)
        failed_requests = len(results) - successful_requests
        
        print("Load Test Results:")
        print("==================")
        print(f"Total Requests: {len(results)}")
        print(f"Successful: {successful_requests}")
        print(f"Failed: {failed_requests}")
        print(f"Success Rate: {(successful_requests / len(results) * 100):.1f}%")
        print(f"Total Time: {total_time:.2f} seconds")
        print(f"Requests/Second: {len(results) / total_time:.2f}")
        print()
        print("Response Time Statistics:")
        print(f"  Mean: {statistics.mean(durations):.3f}s")
        print(f"  Median: {statistics.median(durations):.3f}s")
        print(f"  Min: {min(durations):.3f}s")
        print(f"  Max: {max(durations):.3f}s")
        if len(durations) > 1:
            print(f"  Std Dev: {statistics.stdev(durations):.3f}s")
        
        # Percentiles
        sorted_durations = sorted(durations)
        print("Percentiles:")
        for p in [50, 75, 90, 95, 99]:
            idx = int((p / 100.0) * len(sorted_durations))
            if idx >= len(sorted_durations):
                idx = len(sorted_durations) - 1
            print(f"  {p}th: {sorted_durations[idx]:.3f}s")

async def main():
    tester = LoadTester()
    await tester.run_load_test(concurrent_workers=10, requests_per_worker=100)

if __name__ == "__main__":
    asyncio.run(main())
```

## API Documentation Testing

### Swagger UI Testing

```bash
# Test Swagger UI accessibility
curl -v http://localhost:8080/docs 2>&1 | grep "200 OK" && echo "✅ Swagger UI accessible"

# Test OpenAPI spec validity
curl -s http://localhost:8080/openapi.json | jq . >/dev/null && echo "✅ Valid JSON OpenAPI spec"

# Test ReDoc accessibility
curl -v http://localhost:8080/redoc 2>&1 | grep "200 OK" && echo "✅ ReDoc accessible"
```

### OpenAPI Spec Validation

```python
#!/usr/bin/env python3
# File: validate_openapi.py

import requests
import json
from jsonschema import validate, ValidationError

def validate_openapi_spec(base_url="http://localhost:8080"):
    """Validate OpenAPI specification"""
    try:
        # Fetch OpenAPI spec
        response = requests.get(f"{base_url}/openapi.json")
        response.raise_for_status()
        spec = response.json()
        
        # Basic structure validation
        required_fields = ["openapi", "info", "paths"]
        for field in required_fields:
            if field not in spec:
                print(f"❌ Missing required field: {field}")
                return False
        
        # Version validation
        if not spec.get("openapi", "").startswith("3."):
            print(f"❌ Invalid OpenAPI version: {spec.get('openapi')}")
            return False
        
        # Info validation
        info = spec.get("info", {})
        if not info.get("title") or not info.get("version"):
            print("❌ Missing required info fields (title, version)")
            return False
        
        # Paths validation
        paths = spec.get("paths", {})
        if not paths:
            print("❌ No paths defined")
            return False
        
        print("✅ OpenAPI specification is valid")
        print(f"  Title: {info.get('title')}")
        print(f"  Version: {info.get('version')}")
        print(f"  OpenAPI Version: {spec.get('openapi')}")
        print(f"  Paths: {len(paths)}")
        
        return True
        
    except requests.RequestException as e:
        print(f"❌ Failed to fetch OpenAPI spec: {e}")
        return False
    except json.JSONDecodeError as e:
        print(f"❌ Invalid JSON in OpenAPI spec: {e}")
        return False
    except Exception as e:
        print(f"❌ Validation error: {e}")
        return False

if __name__ == "__main__":
    validate_openapi_spec()
```

## Continuous Testing

### GitHub Actions Workflow

```yaml
# File: .github/workflows/api-tests.yml
name: REST API Tests

on:
  push:
    branches: [ main, develop ]
  pull_request:
    branches: [ main ]

jobs:
  api-tests:
    runs-on: ubuntu-latest
    
    steps:
    - uses: actions/checkout@v3
    
    - name: Install dependencies
      run: |
        sudo apt-get update
        sudo apt-get install -y build-essential cmake pkg-config
        sudo apt-get install -y nlohmann-json3-dev
        
    - name: Build OpenAuto
      run: |
        mkdir -p build
        cd build
        cmake ..
        make -j$(nproc)
        
    - name: Start API server
      run: |
        cd build
        ./autoapp --api-only &
        sleep 5
        
    - name: Run API tests
      run: |
        python3 test_api.py
        ./test_rest_api.sh
        
    - name: Run load tests
      run: |
        python3 comprehensive_load_test.py
```

This comprehensive testing guide provides multiple approaches to validate the OpenAuto REST API server functionality, from quick manual validation to automated testing suites and load testing scenarios.
