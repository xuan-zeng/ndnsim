/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2013-2015 Regents of the University of California.
 *
 * This file is part of ndn-cxx library (NDN C++ library with eXperimental eXtensions).
 *
 * ndn-cxx library is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * ndn-cxx library is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received copies of the GNU General Public License and GNU Lesser
 * General Public License along with ndn-cxx, e.g., in COPYING.md file.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 */

#ifndef NDN_INTEREST_HPP
#define NDN_INTEREST_HPP

#include "common.hpp"

#include "name.hpp"
#include "selectors.hpp"
#include "util/time.hpp"
#include "management/nfd-local-control-header.hpp"
#include "tag-host.hpp"
#include "link.hpp"
#include "encoding/tlv.hpp"

#ifdef NDN_CXX_KITE
// MACRO for new field type value
#define TRACE_FORWARDING_FLAG_TYPE 77
#endif // NDN_CXX_KITE

#define TRACE_NAME_TYPE 78


namespace ndn {

class Data;

/** @var const unspecified_duration_type DEFAULT_INTEREST_LIFETIME;
 *  @brief default value for InterestLifetime
 */
const time::milliseconds DEFAULT_INTEREST_LIFETIME = time::milliseconds(4000);

/** @brief represents an Interest packet
 */
class Interest : public TagHost, public enable_shared_from_this<Interest>
{
public:
  class Error : public tlv::Error
  {
  public:
    explicit
    Error(const std::string& what)
      : tlv::Error(what)
    {
    }
  };

  /** @brief Create a new Interest with an empty name (`ndn:/`)
   *  @warning In certain contexts that use Interest::shared_from_this(), Interest must be created
   *           using `make_shared`. Otherwise, .shared_from_this() will throw an exception.
   */
  Interest();

  /** @brief Create a new Interest with the given name
   *  @param name The name for the interest.
   *  @note This constructor allows implicit conversion from Name.
   *  @warning In certain contexts that use Interest::shared_from_this(), Interest must be created
   *           using `make_shared`. Otherwise, .shared_from_this() will throw an exception.
   */
  Interest(const Name& name);

  /** @brief Create a new Interest with the given name and interest lifetime
   *  @param name             The name for the interest.
   *  @param interestLifetime The interest lifetime in time::milliseconds, or -1 for none.
   *  @warning In certain contexts that use Interest::shared_from_this(), Interest must be created
   *           using `make_shared`. Otherwise, .shared_from_this() will throw an exception.
   */
  Interest(const Name& name, const time::milliseconds& interestLifetime);

  /** @brief Create from wire encoding
   *  @warning In certain contexts that use Interest::shared_from_this(), Interest must be created
   *           using `make_shared`. Otherwise, .shared_from_this() will throw an exception.
   */
  explicit
  Interest(const Block& wire);

  /**
   * @brief Fast encoding or block size estimation
   */
  template<encoding::Tag TAG>
  size_t
  wireEncode(EncodingImpl<TAG>& encoder) const;

  /**
   * @brief Encode to a wire format
   */
  const Block&
  wireEncode() const;

  /**
   * @brief Decode from the wire format
   */
  void
  wireDecode(const Block& wire);

  /**
   * @brief Check if already has wire
   */
  bool
  hasWire() const
  {
    return m_wire.hasWire();
  }

  /**
   * @brief Encode the name according to the NDN URI Scheme
   *
   * If there are interest selectors, this method will append "?" and add the selectors as
   * a query string.  For example, "/test/name?ndn.ChildSelector=1"
   */
  std::string
  toUri() const;

public: // Link and forwarding hint

   /**
   * @brief Check whether the Interest contains a Link object
   * @return True if there is a link object, otherwise false
   */
  bool
  hasLink() const;

#ifdef NDNSIM
  /**
   * @brief Get the link object for this interest
   * @return The link object if there is one contained in this interest
   * @throws Interest::Error if there is no link object contained in the interest
   */
#else
  /**
   * @brief Get the link object for this interest
   * @return The link object if there is one contained in this interest
   * @throws Interest::Error if there is no link object contained in the interest
   * @throws tlv::Error if the incorporated link object is malformed
   */
#endif // NDNSIM
#ifdef NDNSIM
  Link
#else
  const Link&
#endif // NDNSIM
  getLink() const;

  /**
   * @brief Set the link object for this interest
   * @param link The link object that will be included in this interest (in wire format)
   * @post !hasSelectedDelegation()
   */
  void
  setLink(const Block& link);

#ifdef NDNSIM
  /**
   *@brief Reset the wire format of the given interest and the contained link
   */
#else
  /**
   * @brief Delete the link object for this interest
   * @post !hasLink()
   */
#endif // NDNSIM
  void
  unsetLink();

  /**
   * @brief Check whether the Interest includes a selected delegation
   * @return True if there is a selected delegation, otherwise false
   */
  bool
  hasSelectedDelegation() const;

  /**
   * @brief Get the name of the selected delegation
   * @return The name of the selected delegation
   * @throw Error SelectedDelegation is not set.
   */
  Name
  getSelectedDelegation() const;

  /**
   * @brief Set the selected delegation
   * @param delegationName The name of the selected delegation
   * @throw Error Link is not set.
   * @throw std::invalid_argument @p delegationName does not exist in Link.
   */
  void
  setSelectedDelegation(const Name& delegationName);

  /**
   * @brief Set the selected delegation
   * @param delegationIndex The index of the selected delegation
   * @throw Error Link is not set.
   * @throw std::out_of_range @p delegationIndex is out of bound in Link.
   */
  void
  setSelectedDelegation(size_t delegationIndex);

   /**
   * @brief Unset the selected delegation
   */
  void
  unsetSelectedDelegation();

public: // matching
  /** @brief Check if Interest, including selectors, matches the given @p name
   *  @param name The name to be matched. If this is a Data name, it shall contain the
   *              implicit digest component
   */
  bool
  matchesName(const Name& name) const;

  /**
   * @brief Check if Interest can be satisfied by @p data.
   *
   * This method considers Name, MinSuffixComponents, MaxSuffixComponents,
   * PublisherPublicKeyLocator, and Exclude.
   * This method does not consider ChildSelector and MustBeFresh.
   *
   * @todo recognize implicit digest component
   */
  bool
  matchesData(const Data& data) const;

public: // Name and guiders
  const Name&
  getName() const
  {
    return m_name;
  }

  Interest&
  setName(const Name& name)
  {
    m_name = name;
    m_wire.reset();
    return *this;
  }
  
#ifdef NDN_CXX_KITE
  const Name&
  getTraceName() const
  {
    return m_traceName;
  }
 
  Interest&
  setTraceName(const Name& name)
  {
    m_traceName = name;
    m_wire.reset();
    return *this;
  }
  
    uint8_t
  getTraceForwardingFlag() const
  {
    return m_traceForwardingFlag;
  }
  
  Interest&
  setTraceForwardingFlag(uint8_t flag)
  {
    m_traceForwardingFlag = flag;
    m_wire.reset();
    return *this;
  }
#endif // NDN_CXX_KITE
  const time::milliseconds&
  getInterestLifetime() const
  {
    return m_interestLifetime;
  }

  Interest&
  setInterestLifetime(const time::milliseconds& interestLifetime)
  {
    m_interestLifetime = interestLifetime;
    m_wire.reset();
    return *this;
  }

  /** @brief Check if Nonce set
   */
  bool
  hasNonce() const
  {
    return m_nonce.hasWire();
  }

  /** @brief Get Interest's nonce
   *
   *  If nonce was not set before this call, it will be automatically assigned to a random value
   */
  uint32_t
  getNonce() const;

  /** @brief Set Interest's nonce
   *
   *  If wire format already exists, this call simply replaces nonce in the
   *  existing wire format, without resetting and recreating it.
   */
  Interest&
  setNonce(uint32_t nonce);

  /** @brief Refresh nonce
   *
   *  It's guaranteed that new nonce value differs from the existing one.
   *
   *  If nonce is already set, it will be updated to a different random value.
   *  If nonce is not set, this method does nothing.
   */
  void
  refreshNonce();

public: // local control header
  nfd::LocalControlHeader&
  getLocalControlHeader()
  {
    return m_localControlHeader;
  }

  const nfd::LocalControlHeader&
  getLocalControlHeader() const
  {
    return m_localControlHeader;
  }

  uint64_t
  getIncomingFaceId() const
  {
    return getLocalControlHeader().getIncomingFaceId();
  }

  Interest&
  setIncomingFaceId(uint64_t incomingFaceId)
  {
    getLocalControlHeader().setIncomingFaceId(incomingFaceId);
    // ! do not reset Interest's wire !
    return *this;
  }

  uint64_t
  getNextHopFaceId() const
  {
    return getLocalControlHeader().getNextHopFaceId();
  }

  Interest&
  setNextHopFaceId(uint64_t nextHopFaceId)
  {
    getLocalControlHeader().setNextHopFaceId(nextHopFaceId);
    // ! do not reset Interest's wire !
    return *this;
  }

public: // Selectors
  /**
   * @return true if Interest has any selector present
   */
  bool
  hasSelectors() const
  {
    return !m_selectors.empty();
  }

  const Selectors&
  getSelectors() const
  {
    return m_selectors;
  }

  Interest&
  setSelectors(const Selectors& selectors)
  {
    m_selectors = selectors;
    m_wire.reset();
    return *this;
  }

  int
  getMinSuffixComponents() const
  {
    return m_selectors.getMinSuffixComponents();
  }

  Interest&
  setMinSuffixComponents(int minSuffixComponents)
  {
    m_selectors.setMinSuffixComponents(minSuffixComponents);
    m_wire.reset();
    return *this;
  }

  int
  getMaxSuffixComponents() const
  {
    return m_selectors.getMaxSuffixComponents();
  }

  Interest&
  setMaxSuffixComponents(int maxSuffixComponents)
  {
    m_selectors.setMaxSuffixComponents(maxSuffixComponents);
    m_wire.reset();
    return *this;
  }

  const KeyLocator&
  getPublisherPublicKeyLocator() const
  {
    return m_selectors.getPublisherPublicKeyLocator();
  }

  Interest&
  setPublisherPublicKeyLocator(const KeyLocator& keyLocator)
  {
    m_selectors.setPublisherPublicKeyLocator(keyLocator);
    m_wire.reset();
    return *this;
  }

  const Exclude&
  getExclude() const
  {
    return m_selectors.getExclude();
  }

  Interest&
  setExclude(const Exclude& exclude)
  {
    m_selectors.setExclude(exclude);
    m_wire.reset();
    return *this;
  }

  int
  getChildSelector() const
  {
    return m_selectors.getChildSelector();
  }

  Interest&
  setChildSelector(int childSelector)
  {
    m_selectors.setChildSelector(childSelector);
    m_wire.reset();
    return *this;
  }

  int
  getMustBeFresh() const
  {
    return m_selectors.getMustBeFresh();
  }

  Interest&
  setMustBeFresh(bool mustBeFresh)
  {
    m_selectors.setMustBeFresh(mustBeFresh);
    m_wire.reset();
    return *this;
  }
  
#ifdef NDN_CXX_MAPME
  tlv::SpecialInterestTypeValue
  getSpecialInterestType() const
  {
    return m_specialInterestType;
  }
  
  int
  getSequenceNumber() const
  {
    return m_specialInterestSequenceNumber;
  }

  Interest&
  setAsSpecialInterest(tlv::SpecialInterestTypeValue type, int sequenceNumber)
  {
    if (sequenceNumber < -1) {
     throw Error("Sequence number set is not valid, sequence number must be >=-1. Value of sequence number is not modified.");
     return *this;
    }

    m_specialInterestType = type;
    m_wire.reset();

    m_specialInterestSequenceNumber = sequenceNumber;
    m_wire.reset();
    return *this;   
  }
#endif // NDN_CXX_MAPME

  //Natalya
  //Natalya

public: // EqualityComparable concept
  bool
  operator==(const Interest& other) const
  {
    return wireEncode() == other.wireEncode();
  }

  bool
  operator!=(const Interest& other) const
  {
    return !(*this == other);
  }


private:
  Name m_name;
  Selectors m_selectors;
  mutable Block m_nonce;
  time::milliseconds m_interestLifetime;

  mutable Block m_link;
#ifndef NDNSIM
  mutable shared_ptr<Link> m_linkCached;
#endif // !NDNSIM
  size_t m_selectedDelegationIndex;

#ifdef NDN_CXX_MAPME 
  tlv::SpecialInterestTypeValue m_specialInterestType;
  
  /**
   * @brief for the moment it is optional(commented out in the code), a flag to indicate if signature is verified
   * 0 means not verified, 
   * 1 means verified,
   * 255 means no flag (corresponds to situation there is no special interest type)
   */
  //uint8_t m_isVerified;
   /**
    *  @brief the sequence number that indicates the freshness of the special interest.
    *         >=0 indicates the sequence number of the special interest
    *         -1  means no sequence number (corresponds to situation there is no special interest type)
    *         <-1 is not admitted
    */
  int m_specialInterestSequenceNumber;
#endif // NDN_CXX_MAPME

  
#ifdef NDN_CXX_KITE
  /* @brief optional fields for interest trace forwarding:
   *
   * 00000001 traceable
   * 00000010 traceonly
   *
   * Namely, in most cases, we only have:
   * 1=traceable
   * 2=traceonly
   * 3=both traceable and traceonly
   */
  uint8_t m_traceForwardingFlag;  // default to -1, when it is -1, this field is neglected when encoding the packet
  Name m_traceName;
#endif // NDN_CXX_KITE

  //Natalya
  //Natalya

  mutable Block m_wire;

  nfd::LocalControlHeader m_localControlHeader;
  friend class nfd::LocalControlHeader;
};

std::ostream&
operator<<(std::ostream& os, const Interest& interest);

inline std::string
Interest::toUri() const
{
  std::ostringstream os;
  os << *this;
  return os.str();
}

} // namespace ndn

#endif // NDN_INTEREST_HPP
