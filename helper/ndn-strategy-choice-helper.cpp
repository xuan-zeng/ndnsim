/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include "ndn-strategy-choice-helper.hpp"

#include "ns3/log.h"

#include "ndn-stack-helper.hpp"
///#include "strategy-choice.hpp"

namespace ns3 {
namespace ndn {

NS_LOG_COMPONENT_DEFINE("ndn.StrategyChoiceHelper");

void
StrategyChoiceHelper::sendCommand(const ControlParameters& parameters, Ptr<Node> node)
{
  NS_LOG_DEBUG("Strategy choice command was initialized");
  Block encodedParameters(parameters.wireEncode());

  Name commandName("/localhost/nfd/strategy-choice");
  commandName.append("set");
  commandName.append(encodedParameters);

  shared_ptr<Interest> command(make_shared<Interest>(commandName));
  StackHelper::getKeyChain().sign(*command);
  Ptr<L3Protocol> L3protocol = node->GetObject<L3Protocol>();
  auto strategyChoiceManager = L3protocol->getStrategyChoiceManager();
  strategyChoiceManager->onStrategyChoiceRequest(*command);
  NS_LOG_DEBUG("Forwarding strategy installed in node " << node->GetId());
}

void
StrategyChoiceHelper::Install(const NodeContainer& c, const Name& namePrefix, const Name& strategy)
{
  for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
    Install(*i, namePrefix, strategy);
  }
}

void
StrategyChoiceHelper::Install(Ptr<Node> node, const Name& namePrefix, const Name& strategy)
{
  ControlParameters parameters;
  parameters.setName(namePrefix);
  NS_LOG_DEBUG("Node ID: " << node->GetId() << " with forwarding strategy " << strategy);
  parameters.setStrategy(strategy);
  sendCommand(parameters, node);
}

void
StrategyChoiceHelper::InstallAll(const Name& namePrefix, const Name& strategy)
{
  Install(NodeContainer::GetGlobal(), namePrefix, strategy);
}

void
StrategyChoiceHelper::AllowBSRetransmissions(Ptr<Node> node, const Name& strategyName, uint32_t n_rtx)
{
	Ptr<L3Protocol> L3protocol = node->GetObject<L3Protocol>();
	auto strategyChoiceManager = L3protocol->getStrategyChoiceManager();
	nfd::StrategyChoice & strategyChoice = strategyChoiceManager->GetStrategyChoice();
	nfd::fw::Strategy * strategy = strategyChoice.getPublicStrategy(strategyName);
	strategy->SetNRetransmissions(n_rtx);
}

} // namespace ndn

} // namespace ns
