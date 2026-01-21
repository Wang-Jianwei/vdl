# Helper function to discover Catch2 tests
# This function is a no-op if Catch2 is not available
function(catch_discover_tests target)
    # This is a simplified version - full catch_discover_tests would use
    # the Catch2 built-in support for test discovery
    # For now, we rely on standard CTest
endfunction()
