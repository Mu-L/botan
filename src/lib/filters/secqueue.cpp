/*
* SecureQueue
* (C) 1999-2007 Jack Lloyd
*     2012 Markus Wanner
*
* Botan is released under the Simplified BSD License (see license.txt)
*/

#include <botan/internal/secqueue.h>

#include <botan/mem_ops.h>
#include <algorithm>

namespace Botan {

/**
* A node in a SecureQueue
*/
class SecureQueueNode final {
   public:
      SecureQueueNode() : m_next(nullptr), m_buffer(DefaultBufferSize), m_start(0), m_end(0) {}

      ~SecureQueueNode() {
         m_next = nullptr;
         m_start = m_end = 0;
      }

      SecureQueueNode(const SecureQueueNode& other) = delete;
      SecureQueueNode(SecureQueueNode&& other) = delete;
      SecureQueueNode& operator=(const SecureQueueNode& other) = delete;
      SecureQueueNode& operator=(SecureQueueNode&& other) = delete;

      size_t write(const uint8_t input[], size_t length) {
         size_t copied = std::min<size_t>(length, m_buffer.size() - m_end);
         copy_mem(m_buffer.data() + m_end, input, copied);
         m_end += copied;
         return copied;
      }

      size_t read(uint8_t output[], size_t length) {
         size_t copied = std::min(length, m_end - m_start);
         copy_mem(output, m_buffer.data() + m_start, copied);
         m_start += copied;
         return copied;
      }

      size_t peek(uint8_t output[], size_t length, size_t offset = 0) {
         const size_t left = m_end - m_start;
         if(offset >= left) {
            return 0;
         }
         size_t copied = std::min(length, left - offset);
         copy_mem(output, m_buffer.data() + m_start + offset, copied);
         return copied;
      }

      size_t size() const { return (m_end - m_start); }

   private:
      friend class SecureQueue;
      SecureQueueNode* m_next;
      secure_vector<uint8_t> m_buffer;
      size_t m_start, m_end;
};

/*
* Create a SecureQueue
*/
SecureQueue::SecureQueue() : m_bytes_read(0) {
   set_next(nullptr, 0);
   m_head = m_tail = new SecureQueueNode;  // NOLINT(*-owning-memory)
}

/*
* Copy a SecureQueue
*/
SecureQueue::SecureQueue(const SecureQueue& input) : Fanout_Filter(), m_bytes_read(0) {
   set_next(nullptr, 0);

   m_head = m_tail = new SecureQueueNode;  // NOLINT(*-owning-memory)
   SecureQueueNode* temp = input.m_head;
   while(temp != nullptr) {
      write(&temp->m_buffer[temp->m_start], temp->m_end - temp->m_start);
      temp = temp->m_next;
   }
}

/*
* Destroy this SecureQueue
*/
void SecureQueue::destroy() {
   SecureQueueNode* temp = m_head;
   while(temp != nullptr) {
      SecureQueueNode* holder = temp->m_next;
      delete temp;  // NOLINT(*-owning-memory)
      temp = holder;
   }
   m_head = m_tail = nullptr;
}

/*
* Copy a SecureQueue
*/
SecureQueue& SecureQueue::operator=(const SecureQueue& input) {
   if(this == &input) {
      return *this;
   }

   destroy();
   m_bytes_read = input.get_bytes_read();
   m_head = m_tail = new SecureQueueNode;  // NOLINT(*-owning-memory)
   SecureQueueNode* temp = input.m_head;
   while(temp != nullptr) {
      write(&temp->m_buffer[temp->m_start], temp->m_end - temp->m_start);
      temp = temp->m_next;
   }
   return (*this);
}

/*
* Add some bytes to the queue
*/
void SecureQueue::write(const uint8_t input[], size_t length) {
   if(m_head == nullptr) {
      m_head = m_tail = new SecureQueueNode;  // NOLINT(*-owning-memory)
   }
   while(length > 0) {
      const size_t n = m_tail->write(input, length);
      input += n;
      length -= n;
      if(length > 0) {
         m_tail->m_next = new SecureQueueNode;  // NOLINT(*-owning-memory)
         m_tail = m_tail->m_next;
      }
   }
}

/*
* Read some bytes from the queue
*/
size_t SecureQueue::read(uint8_t output[], size_t length) {
   size_t got = 0;
   while(length > 0 && m_head != nullptr) {
      const size_t n = m_head->read(output, length);
      output += n;
      got += n;
      length -= n;
      if(m_head->size() == 0) {
         SecureQueueNode* holder = m_head->m_next;
         delete m_head;  // NOLINT(*-owning-memory)
         m_head = holder;
      }
   }
   m_bytes_read += got;
   return got;
}

/*
* Read data, but do not remove it from queue
*/
size_t SecureQueue::peek(uint8_t output[], size_t length, size_t offset) const {
   SecureQueueNode* current = m_head;

   while(offset > 0 && current != nullptr) {
      if(offset >= current->size()) {
         offset -= current->size();
         current = current->m_next;
      } else {
         break;
      }
   }

   size_t got = 0;
   while(length > 0 && current != nullptr) {
      const size_t n = current->peek(output, length, offset);
      offset = 0;
      output += n;
      got += n;
      length -= n;
      current = current->m_next;
   }
   return got;
}

/**
* Return how many bytes have been read so far.
*/
size_t SecureQueue::get_bytes_read() const {
   return m_bytes_read;
}

/*
* Return how many bytes the queue holds
*/
size_t SecureQueue::size() const {
   SecureQueueNode* current = m_head;
   size_t count = 0;

   while(current != nullptr) {
      count += current->size();
      current = current->m_next;
   }
   return count;
}

/*
* Test if the queue has any data in it
*/
bool SecureQueue::end_of_data() const {
   return (size() == 0);
}

bool SecureQueue::empty() const {
   return (size() == 0);
}

}  // namespace Botan
