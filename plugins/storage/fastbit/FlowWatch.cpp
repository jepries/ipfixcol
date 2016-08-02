/*
 * \file FlowWatch.cpp
 * \author Petr Kramolis <kramolis@cesnet.cz>
 * \brief class for flows and SQ numbers check
 *
 * Copyright (C) 2015 CESNET, z.s.p.o.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name of the Company nor the names of its contributors
 *    may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * ALTERNATIVELY, provided that this notice is retained in full, this
 * product may be distributed under the terms of the GNU General Public
 * License (GPL) version 2 or later, in which case the provisions
 * of the GPL apply INSTEAD OF those given above.
 *
 * This software is provided ``as is, and any express or implied
 * warranties, including, but not limited to, the implied warranties of
 * merchantability and fitness for a particular purpose are disclaimed.
 * In no event shall the company or contributors be liable for any
 * direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute
 * goods or services; loss of use, data, or profits; or business
 * interruption) however caused and on any theory of liability, whether
 * in contract, strict liability, or tort (including negligence or
 * otherwise) arising in any way out of the use of this software, even
 * if advised of the possibility of such damage.
 *
 */

#include <iostream>
#include <fstream>
#include <stdint.h>
#include <sys/stat.h>

#include "FlowWatch.h"

FlowWatch::FlowWatch()
{
	reset_state();
}

void FlowWatch::reset_state()
{
	reset = true;
	recv_flows = 0;
	prev_recv_flows = 0;
	last_seq_no = 0;
	first_seq_no = 0;
}

void FlowWatch::update_seq_no(uint64_t SQ)
{
	if (reset == true) {
		first_seq_no = last_seq_no = SQ;
		reset = false;
	} else {
		if (SQ < first_seq_no) {
			/* Detect SQ reset (modulo 2^32) */
			if (first_seq_no > SQ_TOP_LIMIT && SQ < SQ_BOT_LIMIT) {
				/* Is this first packet with reset SQ? */
				if (last_seq_no < SQ_BOT_LIMIT) {
					if (last_seq_no < SQ) {
						last_seq_no = SQ;
					}
				} else {
					last_seq_no = SQ;
				}
			}

			/* First packet or out of order packet with lesser SQ */
			first_seq_no = SQ;
		}
		if (SQ > last_seq_no) {
			if(last_seq_no < SQ_BOT_LIMIT && SQ > SQ_TOP_LIMIT){
				/* Do nothing; out of order packet with SQ num before SQ reset */
			} else {
				last_seq_no = SQ;
			}
		}
	}
}

void FlowWatch::add_flows(uint64_t recFlows)
{
	prev_recv_flows = recFlows;
	recv_flows += recFlows;
}

uint64_t FlowWatch::exported_flows()
{
	uint expFlows;
	if (last_seq_no < first_seq_no) {
		expFlows = SQ_MAX - first_seq_no;
		expFlows += last_seq_no;
	} else {
		expFlows = last_seq_no - first_seq_no;
	}

	return expFlows + prev_recv_flows;
}

uint64_t FlowWatch::received_flows()
{
	return recv_flows;
}

int FlowWatch::write(std::string dir)
{
	std::ofstream statFile;
	std::ifstream iStatFile;
	std::string fileName, tmp;
	uint64_t exported = 0, received = 0;
	struct stat st;

	/* Check whether directory exists */
	if (stat(dir.c_str(), &st) || !S_ISDIR(st.st_mode)) {
		return 0;
	}

	/* Create filename */
	fileName = dir + "flowsStats.txt";

	iStatFile.open(fileName.c_str(), std::ios_base::in);

	/* Get current stats */
	if (iStatFile.is_open()) {
		/* Read the first two lines containing numbers */
		std::getline(iStatFile, tmp, ':');
		iStatFile >> exported;

		std::getline(iStatFile, tmp, ':');
		iStatFile >> received;

		iStatFile.close();
	}

	/* Write updated stats */
	statFile.open(fileName.c_str(), std::ios_base::trunc | std::ios_base::out);

	exported += exported_flows();
	received += received_flows();

	if (statFile.is_open()) {
		statFile << "Exported flow records: " << exported << std::endl;
		statFile << "Received flow records: " << received << std::endl;
		statFile << "Lost flow records: " << exported - received << std::endl;
		statFile.close();
		return 0;
	}

	return -1;
}

FlowWatch::~FlowWatch() {}
