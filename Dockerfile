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
    make

# Set working directory
WORKDIR /app

# Copy source files
COPY CMakeLists.txt ./
COPY include/ ./include/
COPY src/ ./src/

# Build the project
RUN mkdir -p build && \
    cd build && \
    cmake -DBUILD_TESTING=OFF .. && \
    make

# ==============================
# Stage 2: Runtime
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

# Create data directory for scores
RUN mkdir -p /app/data && chown -R snakeuser:snakeuser /app

# Declare volume for persistence
VOLUME ["/app/data"]

# Switch to non-root user
USER snakeuser

# Run the game by default
CMD ["/app/snake"]
