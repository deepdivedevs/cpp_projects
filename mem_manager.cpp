#include <iostream>
#include <vector>

class MemoryManager {
private:
    int* blocks;
    size_t size;
    bool* isUsed;
    size_t nextFreeBlock;

    bool resize(size_t newSize) {
        if (newSize <= size) {
            std::cout << "New size must be larger\n";
            return false;
        }
        int* newBlocks = new int[newSize];
        bool* newIsUsed = new bool[newSize]{};

        for(size_t i = 0; i < size; i++) {
            newBlocks[i] = blocks[i];
            newIsUsed[i] = isUsed[i];
        }

        for(size_t i = size; i < newSize; i++) {
            newIsUsed[i] = false;
        }

        delete[] blocks;
        delete[] isUsed;

        blocks = newBlocks;
        isUsed = newIsUsed;
        size = newSize;
        return true;
    }

public:
    MemoryManager(size_t numBlocks) {
        blocks = new int[numBlocks];
        isUsed = new bool[numBlocks];
        size = numBlocks;
        nextFreeBlock = 0;
    }
    ~MemoryManager() {
        delete[] blocks;
        delete[] isUsed;
    }
    
    size_t allocate(int value) {
        if (nextFreeBlock >= size) {
            if (!resize(size * 2)) {
                return size_t {};    
            }
        }
        size_t index = nextFreeBlock;
        blocks[index] = value;
        isUsed[index] = true;

        while(nextFreeBlock < size && isUsed[nextFreeBlock]) {
            nextFreeBlock++;
        }

        return index;
    }

    bool free(size_t index) {
        if (index >= size) {
            std::cout << "Illegal access\n";
            return false;
        }
        if (!isBlockUsed(index)) {
            std::cout << "Block not in use\n";
            return false;
        }
        isUsed[index] = false;
        if (index < nextFreeBlock) {
            nextFreeBlock = index;
        }
        return true;
    }

    bool write(size_t index, int value) {
        if (index >= size) {
            std::cout << "Illegal access\n";
            return false;
        }
        if (!isBlockUsed(index)) {
            std::cout << "Block not in use\n";
            return false;
        }
        blocks[index] = value;
        return true;
    }

    bool read(size_t index, int& value) {
        if (index >= size) {
            std::cout << "Illegal access\n";
            return false;
        }
        if (!isBlockUsed(index)) {
            std::cout << "Block not in use\n";
            return false;
        }
        value = blocks[index];
        return true;
    }
    
    bool isBlockUsed(size_t index) const {
        return isUsed[index];
    }
    size_t getSize() const {
        return size;
    }

    size_t getNumFreeBlocks() const {
        size_t count = 0;
        for(size_t i = 0; i < size; i++) {
            if(!isUsed[i]) count++;
        }
        return count;
    }

    bool requestResize(size_t newSize) {
        return resize(newSize);
    }

};

int main() {
    std::cout << "\n=== Test 1: Basic Allocation and Reading ===\n";
    MemoryManager mm(3);
    size_t idx1 = mm.allocate(42);
    size_t idx2 = mm.allocate(73);
    int value;
    mm.read(idx1, value);
    std::cout << "Value at idx1: " << value << " (expected 42)\n";
    mm.read(idx2, value);
    std::cout << "Value at idx2: " << value << " (expected 73)\n";

    std::cout << "\n=== Test 2: Writing ===\n";
    mm.write(idx1, 100);
    mm.read(idx1, value);
    std::cout << "Value after write: " << value << " (expected 100)\n";

    std::cout << "\n=== Test 3: Freeing and Reusing ===\n";
    std::cout << "Free blocks before: " << mm.getNumFreeBlocks() << "\n";
    mm.free(idx1);
    std::cout << "Free blocks after freeing: " << mm.getNumFreeBlocks() << "\n";
    size_t idx3 = mm.allocate(999);  // Should reuse idx1's spot
    mm.read(idx3, value);
    std::cout << "Value at reused block: " << value << " (expected 999)\n";

    std::cout << "\n=== Test 4: Error Cases ===\n";
    std::cout << "Trying to read freed block:\n";
    mm.read(idx1, value);  // Should fail
    std::cout << "Trying to write to invalid index:\n";
    mm.write(99, 42);  // Should fail
    std::cout << "Trying to free invalid index:\n";
    mm.free(99);  // Should fail
    std::cout << "Trying to free already freed block:\n";
    mm.free(idx1);  // Should fail

    std::cout << "\n=== Test 5: Auto-Resizing ===\n";
    std::cout << "Initial size: " << mm.getSize() << "\n";
    // Fill it up to force resize
    for(int i = 0; i < 5; i++) {
        size_t idx = mm.allocate(i);
        std::cout << "Allocated " << i << " at index " << idx << "\n";
    }
    std::cout << "Size after auto-resize: " << mm.getSize() << "\n";

    std::cout << "\n=== Test 6: Manual Resizing ===\n";
    std::cout << "Current size: " << mm.getSize() << "\n";
    mm.requestResize(20);
    std::cout << "Size after manual resize: " << mm.getSize() << "\n";
    // Verify old data is intact
    mm.read(idx2, value);
    std::cout << "Value at idx2 after resize: " << value << " (should be unchanged)\n";

    std::cout << "\n=== Test 7: Stress Test ===\n";
    MemoryManager mm2(2);
    std::vector<size_t> indices;
    for(int i = 0; i < 10; i++) {
        indices.push_back(mm2.allocate(i));
        std::cout << "Allocated " << i << " at index " << indices.back() << "\n";
    }
    // Free every other block
    for(size_t i = 0; i < indices.size(); i += 2) {
        mm2.free(indices[i]);
        std::cout << "Freed index " << indices[i] << "\n";
    }
    std::cout << "Free blocks after stress test: " << mm2.getNumFreeBlocks() << "\n";

    return 0;
}