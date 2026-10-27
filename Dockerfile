# Snake Game Dockerfile
# Author: Miguel Mochizuki Silva
# Description: Multi-stage Dockerfile with Alpine for minimal image size

# ==============================
# Stage 1: Builder
# ==============================

ARG TARGETPLATFORM
FROM --platform=$TARGETPLATFORM alpine:latest AS builder

# Install build dependencies
RUN apk add --no-cache \
    build-base \
    cmake \
    g++ \
    make \
    python3

# Set working directory
WORKDIR /app

# e2e timeout multiplier (slow under emulation)
ARG SNAKE_E2E_TIMEOUT_SCALE=10

# Copy source files
COPY CMakeLists.txt .gitattributes ./
COPY include/ ./include/
COPY src/ ./src/
COPY tests/ ./tests/

# Build the project
RUN mkdir -p build && \
    cd build && \
    cmake -DBUILD_TESTING=ON -DSNAKE_E2E_TIMEOUT_SCALE=${SNAKE_E2E_TIMEOUT_SCALE} .. && \
    cmake --build .

# ==============================
# Stage 2: Tests (gate for runtime image)
# ==============================

FROM builder AS test

# Second ctest run fails loudly if no e2e tests were registered
RUN ctest --test-dir build --output-on-failure -LE e2e && \
    ctest --test-dir build -L e2e --no-tests=error --output-on-failure && \
    touch /tests-passed

# ==============================
# Stage 3: Runtime
# ==============================

ARG TARGETPLATFORM
FROM --platform=$TARGETPLATFORM alpine:latest

# Install runtime dependencies
RUN apk add --no-cache \
    libstdc++

# Create game user for security
RUN adduser -D -s /bin/sh snakeuser

# Set working directory
WORKDIR /app

# Copy executable from builder stage
COPY --from=builder /app/build/bin/snake /app/snake

# Forces the test stage to run (and pass) before this image can be built
COPY --from=test /tests-passed /tmp/tests-passed

# Create data directory for scores
RUN mkdir -p /app/data && chown -R snakeuser:snakeuser /app

# Declare volume for persistence
VOLUME ["/app/data"]

# Switch to non-root user
USER snakeuser

# Run the game by default
CMD ["/app/snake"]
