/*
 * =============================================================================
 *   HSA Runtime Conformance Release License
 * =============================================================================
 * The University of Illinois/NCSA
 * Open Source License (NCSA)
 *
 * Copyright (c) 2014, Advanced Micro Devices, Inc.
 * All rights reserved.
 *
 * Developed by:
 *
 *                 AMD Research and AMD HSA Software Development
 *
 *                 Advanced Micro Devices, Inc.
 *
 *                 www.amd.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal with the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimers.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimers in
 *    the documentation and/or other materials provided with the distribution.
 *  - Neither the names of <Name of Development Group, Name of Institution>,
 *    nor the names of its contributors may be used to endorse or promote
 *    products derived from this Software without specific prior written
 *    permission.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS WITH THE SOFTWARE.
 *
 */

#include <string.h>
#include "queue_utils.h"

void enqueue_dispatch_packet(hsa_queue_t *queue,
                             hsa_kernel_dispatch_packet_t *packet) {
    // Reserve the write_index for the packet
    uint64_t write_index = hsa_queue_add_write_index_relaxed(queue, 1);

    // Enqueue the packet at the location
    enqueue_dispatch_packet_at(write_index, queue, packet);

    return;
}

void enqueue_dispatch_packet_at(uint64_t write_index,
                             hsa_queue_t *queue,
                             hsa_kernel_dispatch_packet_t *packet) {
    // Block until the queue has an empty packet slot
    uint64_t delta;
    do {
        delta = write_index - hsa_queue_load_read_index_relaxed(queue);
    } while (delta > queue->size);

    const uint32_t queue_mask = queue->size - 1;

    hsa_kernel_dispatch_packet_t* packet_base
              = (hsa_kernel_dispatch_packet_t*) &((hsa_kernel_dispatch_packet_t*)(queue->base_address))[write_index&queue_mask];

    // Copy over the packet information
    memcpy(&packet_base->setup, &packet->setup, sizeof(hsa_kernel_dispatch_packet_t) - sizeof(packet_base->header));

    // Atomically set the packet header
    __atomic_store_n((uint16_t*) packet_base, packet->header, __ATOMIC_RELEASE);

    // Ring the doorbell.
    hsa_signal_store_relaxed(queue->doorbell_signal, write_index);

    return;
}

void enqueue_dispatch_packets(hsa_queue_t *queue,
                              uint32_t packet_count,
                              hsa_kernel_dispatch_packet_t packet[]) {
    if (packet_count > 0) {
        // Reserve the write_index for the packet
        uint64_t write_index = hsa_queue_add_write_index_relaxed(queue, packet_count);

        // Dispatch the packets
        enqueue_dispatch_packets_at(write_index, queue, packet_count, packet);
    }

    return;
}

void enqueue_dispatch_packets_at(uint64_t write_index,
                                 hsa_queue_t *queue,
                                 uint32_t packet_count,
                                 hsa_kernel_dispatch_packet_t packet[]) {
    if (packet_count > 0) {
        // Block until the queue has packet_count empty packet slots
        while (packet_count > 0) {
            uint32_t dispatch_count = (packet_count > queue->size) ? queue->size : packet_count;

            uint64_t delta;
            do {
                delta = write_index + dispatch_count - hsa_queue_load_read_index_relaxed(queue);
            } while (delta > queue->size);

            const uint32_t queue_mask = queue->size - 1;

            for (uint32_t i = 0; i < dispatch_count; ++i) {
                hsa_kernel_dispatch_packet_t* packet_base
                          = (hsa_kernel_dispatch_packet_t*) &((hsa_kernel_dispatch_packet_t*)(queue->base_address))[write_index&queue_mask];

                // Copy over the packet information
                memcpy(&packet_base->setup, &packet[i].setup, sizeof(hsa_kernel_dispatch_packet_t) - sizeof(packet_base->header));

                // Atomically set the packet header
                __atomic_store_n((uint16_t*) packet_base, packet[i].header, __ATOMIC_RELEASE);

                ++write_index;
            }

            // Ring the doorbell.
            hsa_signal_store_relaxed(queue->doorbell_signal, write_index);

            // Decrement the packet count
            packet_count -= dispatch_count;
        }
    }

    return;
}
