//
// Created by Lorenzo P on 4/20/26.
//

#ifndef POLYMARKET_GAMMA_TYPES_H
#define POLYMARKET_GAMMA_TYPES_H

#pragma once

#include <optional>
#include <string>
#include <vector>
#include <boost/describe.hpp>
#include <boost/json.hpp>
#include <boost/url.hpp>

#include "url_utils.h"

namespace polymarket::gamma {
    struct FeeSchedule {
        double exponent   = 0.0;
        double rate       = 0.0;
        bool   takerOnly  = false;
        double rebateRate = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(FeeSchedule, (), (exponent, rate, takerOnly, rebateRate))

    struct ImageOptimized {
        std::string  id;
        std::string  imageUrlSource;
        std::string  imageUrlOptimized;
        std::int64_t imageSizeKbSource      = 0;
        std::int64_t imageSizeKbOptimized   = 0;
        bool         imageOptimizedComplete = false;
        std::string  imageOptimizedLastUpdated;
        std::int64_t relID = 0;
        std::string  field;
        std::string  relname;
    };
    BOOST_DESCRIBE_STRUCT(ImageOptimized, (),
        (id, imageUrlSource, imageUrlOptimized,
         imageSizeKbSource, imageSizeKbOptimized, imageOptimizedComplete,
         imageOptimizedLastUpdated, relID, field, relname))

    struct Category {
        std::string id;
        std::string label;
        std::string parentCategory;
        std::string slug;
        std::string publishedAt;
        std::string createdBy;
        std::string updatedBy;
        std::string createdAt;
        std::string updatedAt;
    };
    BOOST_DESCRIBE_STRUCT(Category, (),
        (id, label, parentCategory, slug, publishedAt,
         createdBy, updatedBy, createdAt, updatedAt))

    struct Tag {
        std::string  id;
        std::string  label;
        std::string  slug;
        bool         forceShow = false;
        std::string  publishedAt;
        std::int64_t createdBy = 0;
        std::int64_t updatedBy = 0;
        std::string  createdAt;
        std::string  updatedAt;
        bool         forceHide  = false;
        bool         isCarousel = false;
        bool         requiresTranslation = false;
    };
    BOOST_DESCRIBE_STRUCT(Tag, (),
        (id, label, slug, forceShow, publishedAt,
         createdBy, updatedBy, createdAt, updatedAt,
         forceHide, isCarousel, requiresTranslation))

    struct Chat {
        std::string id;
        std::string channelId;
        std::string channelName;
        std::string channelImage;
        bool        live = false;
        std::string startTime;
        std::string endTime;
    };
    BOOST_DESCRIBE_STRUCT(Chat, (),
        (id, channelId, channelName, channelImage, live, startTime, endTime))

    struct Collection {
        std::string    id;
        std::string    ticker;
        std::string    slug;
        std::string    title;
        std::string    subtitle;
        std::string    collectionType;
        std::string    description;
        std::string    tags;
        std::string    image;
        std::string    icon;
        std::string    headerImage;
        std::string    layout;
        bool           active     = false;
        bool           closed     = false;
        bool           archived   = false;
        bool           isNew      = false;   // JSON key: "new"
        bool           featured   = false;
        bool           restricted = false;
        bool           isTemplate = false;
        std::string    templateVariables;
        std::string    publishedAt;
        std::string    createdBy;
        std::string    updatedBy;
        std::string    createdAt;
        std::string    updatedAt;
        bool           commentsEnabled = false;
        ImageOptimized imageOptimized;
        ImageOptimized iconOptimized;
        ImageOptimized headerImageOptimized;
    };
    BOOST_DESCRIBE_STRUCT(Collection, (),
        (id, ticker, slug, title, subtitle, collectionType, description, tags,
         image, icon, headerImage, layout,
         active, closed, archived, isNew, featured, restricted, isTemplate,
         templateVariables, publishedAt, createdBy, updatedBy, createdAt, updatedAt,
         commentsEnabled, imageOptimized, iconOptimized, headerImageOptimized))

    struct EventCreator {
        std::string id;
        std::string creatorName;
        std::string creatorHandle;
        std::string creatorUrl;
        std::string creatorImage;
        std::string createdAt;
        std::string updatedAt;
    };
    BOOST_DESCRIBE_STRUCT(EventCreator, (),
        (id, creatorName, creatorHandle, creatorUrl, creatorImage,
         createdAt, updatedAt))

    struct EventTemplate {
        std::string id;
        std::string eventTitle;
        std::string eventSlug;
        std::string eventImage;
        std::string marketTitle;
        std::string description;
        std::string resolutionSource;
        bool        negRisk          = false;
        std::string sortBy;
        bool        showMarketImages = false;
        std::string seriesSlug;
        std::string outcomes;
    };
    BOOST_DESCRIBE_STRUCT(EventTemplate, (),
        (id, eventTitle, eventSlug, eventImage, marketTitle, description,
         resolutionSource, negRisk, sortBy, showMarketImages, seriesSlug,
         outcomes))

    // ---------------------------------------------------------------------------
    // Series <-> Event cycle.  Event is forward-declared so Series can hold a
    // std::vector<Event> (legal for incomplete T since C++17, LWG 2185).
    // ---------------------------------------------------------------------------

    struct Event;
    struct Market;

    struct Series {
        std::string  id;
        std::string  ticker;
        std::string  slug;
        std::string  title;
        std::string  subtitle;
        std::string  seriesType;
        std::string  recurrence;
        std::string  description;
        std::string  image;
        std::string  icon;
        std::string  layout;
        bool         active     = false;
        bool         closed     = false;
        bool         archived   = false;
        bool         isNew      = false;     // JSON key: "new"
        bool         featured   = false;
        bool         restricted = false;
        bool         isTemplate = false;
        bool         templateVariables = false;   // bool here; string elsewhere
        std::string  publishedAt;
        std::string  createdBy;
        std::string  updatedBy;
        std::string  createdAt;
        std::string  updatedAt;
        bool         commentsEnabled = false;
        std::string  competitive;                  // string here; double elsewhere
        double       volume24hr = 0.0;
        double       volume     = 0.0;
        double       liquidity  = 0.0;
        std::string  startDate;
        std::string  pythTokenID;
        std::string  cgAssetName;
        std::int64_t score = 0;

        std::vector<Event>      events;
        std::vector<Collection> collections;
        std::vector<Category>   categories;
        std::vector<Tag>        tags;

        std::int64_t      commentCount = 0;
        std::vector<Chat> chats;
        bool              requiresTranslation = false;
    };
    BOOST_DESCRIBE_STRUCT(Series, (),
        (id, ticker, slug, title, subtitle, seriesType, recurrence, description,
         image, icon, layout,
         active, closed, archived, isNew, featured, restricted, isTemplate,
         templateVariables,
         publishedAt, createdBy, updatedBy, createdAt, updatedAt,
         commentsEnabled, competitive, volume24hr, volume, liquidity,
         startDate, pythTokenID, cgAssetName, score,
         events, collections, categories, tags,
         commentCount, chats, requiresTranslation))

    struct EventBase {
        std::string  id;
        std::string  ticker;
        std::string  slug;
        std::string  title;
        std::string  subtitle;
        std::string  description;
        std::string  resolutionSource;
        std::string  startDate;
        std::string  creationDate;
        std::string  endDate;
        std::string  image;
        std::string  icon;
        bool         active     = false;
        bool         closed     = false;
        bool         archived   = false;
        bool         isNew      = false;     // JSON key: "new"
        bool         featured   = false;
        bool         restricted = false;
        double       liquidity    = 0.0;
        double       volume       = 0.0;
        double       openInterest = 0.0;
        std::string  sortBy;
        std::string  category;
        std::string  subcategory;
        bool         isTemplate = false;
        std::string  templateVariables;
        std::string  published_at;                 // snake_case per schema
        std::string  createdBy;
        std::string  updatedBy;
        std::string  createdAt;
        std::string  updatedAt;
        bool         commentsEnabled = false;
        double       competitive = 0.0;
        double       volume24hr = 0.0;
        double       volume1wk  = 0.0;
        double       volume1mo  = 0.0;
        double       volume1yr  = 0.0;
        std::string  featuredImage;
        std::string  disqusThread;
        std::string  parentEvent;
        bool         enableOrderBook = false;
        double       liquidityAmm   = 0.0;
        double       liquidityClob  = 0.0;
        bool         negRisk        = false;
        std::string  negRiskMarketID;
        std::int64_t negRiskFeeBips = 0;
        std::int64_t commentCount   = 0;

        ImageOptimized imageOptimized;
        ImageOptimized iconOptimized;
        ImageOptimized featuredImageOptimized;

        std::vector<std::string> subEvents;
        std::vector<Market>       markets;          // live API currently returns an array here
        std::vector<Series>      series;
        std::vector<Category>    categories;
        std::vector<Collection>  collections;
        std::vector<Tag>         tags;

        bool         cyom                  = false;
        std::string  closedTime;
        bool         showAllOutcomes       = false;
        bool         showMarketImages      = false;
    };
    BOOST_DESCRIBE_STRUCT(EventBase, (),
        (id, ticker, slug, title, subtitle, description, resolutionSource,
         startDate, creationDate, endDate, image, icon,
         active, closed, archived, isNew, featured, restricted,
         liquidity, volume, openInterest, sortBy, category, subcategory,
         isTemplate, templateVariables, published_at,
         createdBy, updatedBy, createdAt, updatedAt,
         commentsEnabled, competitive,
         volume24hr, volume1wk, volume1mo, volume1yr,
         featuredImage, disqusThread, parentEvent,
         enableOrderBook, liquidityAmm, liquidityClob,
         negRisk, negRiskMarketID, negRiskFeeBips, commentCount,
         imageOptimized, iconOptimized, featuredImageOptimized,
         subEvents, markets, series, categories, collections, tags,
         cyom, closedTime, showAllOutcomes, showMarketImages))

    struct Event : EventBase {
        bool         automaticallyResolved = false;
        bool         enableNegRisk         = false;
        bool         automaticallyActive   = false;
        std::string  eventDate;
        std::string  startTime;
        std::int64_t eventWeek = 0;
        std::string  seriesSlug;
        std::string  score;                        // string here; double in Market
        std::string  elapsed;
        std::string  period;
        bool         live  = false;
        bool         ended = false;
        std::string  finishedTimestamp;
        std::string  gmpChartMode;

        std::vector<EventCreator>  eventCreators;
        std::int64_t               tweetCount = 0;
        std::vector<Chat>          chats;
        std::int64_t               featuredOrder = 0;
        bool                       estimateValue = false;
        bool                       cantEstimate  = false;
        std::string                estimatedValue;
        std::vector<EventTemplate> templates;

        double       spreadsMainLine = 0.0;
        double       totalsMainLine  = 0.0;
        std::string  carouselMap;
        bool         pendingDeployment = false;
        bool         deploying         = false;
        std::string  deployingTimestamp;
        std::string  scheduledDeploymentTimestamp;
        std::string  gameStatus;
        std::string  parentEventId;
        std::string  sportsradarMatchId;
        std::string  turnProviderId;
        bool         requiresTranslation = false;
        bool         negRiskAugmented = false;
        std::int64_t gameId = 0;
        std::string  electionType;
        std::string  countryName;
        std::string  color;
        bool         cumulativeMarkets = false;
        std::string  awayTeamName;
        std::string  homeTeamName;
    };
    BOOST_DESCRIBE_STRUCT(Event, (EventBase),
        (automaticallyResolved, enableNegRisk, automaticallyActive,
         eventDate, startTime, eventWeek, seriesSlug,
         score, elapsed, period, live, ended,
         finishedTimestamp, gmpChartMode,
         eventCreators, tweetCount, chats, featuredOrder,
         estimateValue, cantEstimate, estimatedValue, templates,
         spreadsMainLine, totalsMainLine, carouselMap,
         pendingDeployment, deploying, deployingTimestamp,
         scheduledDeploymentTimestamp, gameStatus,
         parentEventId, sportsradarMatchId, turnProviderId,
         requiresTranslation, negRiskAugmented, gameId,
         electionType, countryName, color, cumulativeMarkets,
         awayTeamName, homeTeamName))

    // ---------------------------------------------------------------------------
    // Market (top-level element of the markets[] array)
    // ---------------------------------------------------------------------------

    struct ClobReward {
        std::string id;
        std::string assetAddress;
        std::string conditionId;
        std::string startDate;
        std::string endDate;
        double      rewardsAmount    = 0.0;
        double      rewardsDailyRate = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(ClobReward, (),
        (id, assetAddress, conditionId, startDate, endDate,
         rewardsAmount, rewardsDailyRate))

    struct MarketBase {
        std::string  id;
        std::string  question;
        std::string  conditionId;
        std::string  slug;
        std::string  twitterCardImage;
        std::string  resolutionSource;
        std::string  endDate;
        std::string  category;
        std::string  ammType;
        std::string  liquidity;             // quoted on the wire
        std::string  sponsorName;
        std::string  sponsorImage;
        std::string  startDate;
        std::string  xAxisValue;
        std::string  yAxisValue;
        std::string  denominationToken;
        std::string  fee;                   // quoted on the wire
        std::string  image;
        std::string  icon;
        std::string  lowerBound;
        std::string  upperBound;
        std::string  description;
        std::string  outcomes;              // JSON-encoded array in a string
        std::string  outcomePrices;         // JSON-encoded array in a string
        std::string  volume;                // quoted on the wire
        bool         active = false;
        std::string  marketType;
        std::string  formatType;
        std::string  lowerBoundDate;
        std::string  upperBoundDate;
        bool         closed = false;
        std::string  marketMakerAddress;
        std::int64_t createdBy = 0;
        std::int64_t updatedBy = 0;
        std::string  createdAt;
        std::string  updatedAt;
        std::string  closedTime;
        bool         wideFormat = false;
        bool         isNew      = false;    // JSON key: "new"
        std::string  mailchimpTag;
        bool         featured   = false;
        bool         archived   = false;
        std::string  resolvedBy;
        bool         restricted = false;
        std::int64_t marketGroup = 0;
        std::string  groupItemTitle;
        std::string  groupItemThreshold;
        std::string  questionID;
        std::string  umaEndDate;
        bool         enableOrderBook       = false;
        double       orderPriceMinTickSize = 0.0;
        double       orderMinSize          = 0.0;
        std::string  umaResolutionStatus;
        std::int64_t curationOrder  = 0;
        double       volumeNum      = 0.0;
        double       liquidityNum   = 0.0;
        std::string  endDateIso;
        std::string  startDateIso;
        std::string  umaEndDateIso;
        bool         hasReviewedDates = false;
        bool         readyForCron     = false;
        bool         commentsEnabled  = false;
        double       volume24hr = 0.0;
        double       volume1wk  = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(MarketBase, (),
        (id, question, conditionId, slug, twitterCardImage, resolutionSource,
         endDate, category, ammType, liquidity, sponsorName, sponsorImage,
         startDate, xAxisValue, yAxisValue, denominationToken, fee, image, icon,
         lowerBound, upperBound, description, outcomes, outcomePrices, volume,
         active, marketType, formatType, lowerBoundDate, upperBoundDate, closed,
         marketMakerAddress, createdBy, updatedBy, createdAt, updatedAt, closedTime,
         wideFormat, isNew, mailchimpTag, featured, archived, resolvedBy, restricted,
         marketGroup, groupItemTitle, groupItemThreshold, questionID, umaEndDate,
         enableOrderBook, orderPriceMinTickSize, orderMinSize,
         umaResolutionStatus, curationOrder, volumeNum, liquidityNum,
         endDateIso, startDateIso, umaEndDateIso,
         hasReviewedDates, readyForCron, commentsEnabled,
         volume24hr, volume1wk))

    struct MarketExt : MarketBase {
        double       volume1mo  = 0.0;
        double       volume1yr  = 0.0;
        std::string  gameStartTime;
        std::int64_t secondsDelay = 0;
        std::string  clobTokenIds;          // JSON-encoded array of token ids
        std::string  disqusThread;
        std::string  shortOutcomes;
        std::string  teamAID;
        std::string  teamBID;
        std::string  umaBond;
        std::string  umaReward;
        bool         fpmmLive = false;
        double       volume24hrAmm = 0.0;
        double       volume1wkAmm  = 0.0;
        double       volume1moAmm  = 0.0;
        double       volume1yrAmm  = 0.0;
        double       volume24hrClob = 0.0;
        double       volume1wkClob  = 0.0;
        double       volume1moClob  = 0.0;
        double       volume1yrClob  = 0.0;
        double       volumeAmm      = 0.0;
        double       volumeClob     = 0.0;
        double       liquidityAmm   = 0.0;
        double       liquidityClob  = 0.0;
        double       makerBaseFee   = 0.0;
        double       takerBaseFee   = 0.0;
        std::int64_t customLiveness = 0;
        bool         acceptingOrders      = false;
        bool         notificationsEnabled = false;
        double       score = 0.0;

        ImageOptimized imageOptimized;
        ImageOptimized iconOptimized;

        std::vector<Event>    events;
        std::vector<Category> categories;
        std::vector<Tag>      tags;

        std::string  creator;
        bool         ready  = false;
        bool         funded = false;
        std::string  pastSlugs;
    };
    BOOST_DESCRIBE_STRUCT(MarketExt, (MarketBase),
        (volume1mo, volume1yr,
         gameStartTime, secondsDelay, clobTokenIds, disqusThread, shortOutcomes,
         teamAID, teamBID, umaBond, umaReward, fpmmLive,
         volume24hrAmm, volume1wkAmm, volume1moAmm, volume1yrAmm,
         volume24hrClob, volume1wkClob, volume1moClob, volume1yrClob,
         volumeAmm, volumeClob, liquidityAmm, liquidityClob,
         makerBaseFee, takerBaseFee, customLiveness,
         acceptingOrders, notificationsEnabled, score,
         imageOptimized, iconOptimized, events, categories, tags,
         creator, ready, funded, pastSlugs))

    struct Market : MarketExt {
        std::string  readyTimestamp;
        std::string  fundedTimestamp;
        std::string  acceptingOrdersTimestamp;
        double       competitive       = 0.0;
        double       rewardsMinSize    = 0.0;
        double       rewardsMaxSpread  = 0.0;
        double       spread            = 0.0;
        bool         automaticallyResolved = false;
        double       oneDayPriceChange   = 0.0;
        double       oneHourPriceChange  = 0.0;
        double       oneWeekPriceChange  = 0.0;
        double       oneMonthPriceChange = 0.0;
        double       oneYearPriceChange  = 0.0;
        double       lastTradePrice      = 0.0;
        double       bestBid             = 0.0;
        double       bestAsk             = 0.0;
        bool         automaticallyActive = false;
        bool         clearBookOnStart    = false;
        std::string  chartColor;
        std::string  seriesColor;
        bool         showGmpSeries    = false;
        bool         showGmpOutcome   = false;
        bool         manualActivation = false;
        bool         negRiskOther     = false;
        std::string  gameId;
        std::string  groupItemRange;
        std::string  sportsMarketType;
        double       line = 0.0;
        std::string  umaResolutionStatuses;
        bool         pendingDeployment = false;
        bool         deploying         = false;
        std::string  deployingTimestamp;
        std::string  scheduledDeploymentTimestamp;
        bool         rfqEnabled = false;
        std::string  eventStartTime;
        bool         feesEnabled = false;
        FeeSchedule  feeSchedule;
        std::int64_t makerRebatesFeeShareBps = 0;
        bool         requiresTranslation = false;
        bool         pagerDutyNotificationEnabled = false;
        bool         approved = false;
        bool         cyom = false;
        bool         holdingRewardsEnabled = false;
        bool         negRisk = false;
        std::string  negRiskRequestID;
        std::string  negRiskMarketID;
        bool         sentDiscord = false;
        std::int64_t twitterCardLastRefreshed = 0;
        std::string  twitterCardLocation;
        std::string  twitterCardLastValidated;
        std::string  categoryMailchimpTag;
        std::string  submittedBy;
        std::string  subcategory;
        std::vector<ClobReward> clobRewards;
    };
    BOOST_DESCRIBE_STRUCT(Market, (MarketExt),
        (readyTimestamp, fundedTimestamp, acceptingOrdersTimestamp,
         competitive, rewardsMinSize, rewardsMaxSpread, spread,
         automaticallyResolved,
         oneDayPriceChange, oneHourPriceChange, oneWeekPriceChange,
         oneMonthPriceChange, oneYearPriceChange,
         lastTradePrice, bestBid, bestAsk,
         automaticallyActive, clearBookOnStart, chartColor, seriesColor,
         showGmpSeries, showGmpOutcome, manualActivation, negRiskOther,
         gameId, groupItemRange, sportsMarketType, line,
         umaResolutionStatuses, pendingDeployment, deploying,
         deployingTimestamp, scheduledDeploymentTimestamp,
         rfqEnabled, eventStartTime, feesEnabled, feeSchedule,
         makerRebatesFeeShareBps, requiresTranslation,
         pagerDutyNotificationEnabled, approved, cyom,
         holdingRewardsEnabled, negRisk, negRiskRequestID, negRiskMarketID,
         sentDiscord, twitterCardLastRefreshed, twitterCardLocation,
         twitterCardLastValidated, categoryMailchimpTag, submittedBy,
         subcategory, clobRewards))

    // ---------------------------------------------------------------------------
    // Response envelope
    // ---------------------------------------------------------------------------

    struct MarketsResponse {
        std::vector<Market> markets;
        std::string         next_cursor;
    };
    BOOST_DESCRIBE_STRUCT(MarketsResponse, (), (markets, next_cursor))

    struct EventsResponse {
        std::vector<Event> events;
        std::string  next_cursor;
    };
    BOOST_DESCRIBE_STRUCT(EventsResponse, (), (events, next_cursor))

    struct Holder {
        std::string proxyWallet;
        std::string bio;
        std::string asset;
        std::string pseudonym;
        double amount;
        bool displayUsernamePublic;
        int outcomeIndex;
        std::string name;
        std::string profileImage;
        std::string profileImageOptimized;
        bool verified;
    };
    BOOST_DESCRIBE_STRUCT(Holder, (), (proxyWallet, bio, asset, pseudonym, amount, displayUsernamePublic, outcomeIndex, name, profileImage, profileImageOptimized, verified))

    struct MarketHolders {
        std::string token;
        std::vector<Holder> holders;
    };
    BOOST_DESCRIBE_STRUCT(MarketHolders, (), (token, holders))

    struct MarketOpenInterest {
        std::string market;
        double value;
    };
    BOOST_DESCRIBE_STRUCT(MarketOpenInterest, (), (market, value))

    struct EventVolume {
        double total;
        std::vector<MarketOpenInterest> markets;
    };
    BOOST_DESCRIBE_STRUCT(EventVolume, (), (total, markets))

    struct MarketParameters {
        int limit = 1000;
        std::vector<std::string> order;
        std::optional<bool> ascending;
        std::vector<int> id;
        std::vector<std::string> slug;
        std::optional<bool> closed;
        std::vector<std::string> clob_token_ids;
        std::vector<std::string> question_ids;
        std::vector<std::string> mkt_maker_address;
        std::optional<int> liquidity_num_min;
        std::optional<int> liquidity_num_max;
        std::optional<int> volume_num_min;
        std::optional<int> volume_num_max;
        std::string start_date_min;
        std::string start_date_max;
        std::string end_date_min;
        std::string end_date_max;
        std::vector<std::string> tag_id;
        std::optional<bool> related_tags;
        std::string game_id;
        std::optional<bool> include_tag;
    };

    struct EventParameters {
        int limit = 20;
        std::string order;
        std::optional<bool> ascending = true;
        std::string after_cursor;

        std::vector<int> id;
        std::vector<std::string> slug;

        // Status Flags
        std::optional<bool> closed;
        std::optional<bool> live;
        std::optional<bool> featured;
        std::optional<bool> cyom;

        // Search & Metrics
        std::string title_search;
        std::optional<double> liquidity_min;
        std::optional<double> liquidity_max;
        std::optional<double> volume_min;
        std::optional<double> volume_max;

        // Dates & Times (ISO 8601 Strings)
        std::string start_date_min;
        std::string start_date_max;
        std::string end_date_min;
        std::string end_date_max;
        std::string start_time_min;
        std::string start_time_max;
        std::string event_date;

        // Tags & Groupings
        std::vector<int> tag_id;
        std::string tag_slug;
        std::vector<int> exclude_tag_id; // Cannot overlap with tag_id
        std::optional<bool> related_tags;
        std::string tag_match;
        std::vector<int> series_id;
        std::vector<int> game_id;

        // Event Specifics
        std::optional<int> event_week;
        std::optional<bool> featured_order;
        std::string recurrence;
        std::vector<std::string> created_by;
        std::optional<int> parent_event_id;
        std::optional<bool> include_children;
        std::string partner_slug;

        // Relation Inclusions
        std::optional<bool> include_chat;
        std::optional<bool> include_template;
        std::optional<bool> include_best_lines;

        // Localization
        std::string locale;
    };

    // ---------------------------------------------------------------------------
    // Teams / Sports
    // ---------------------------------------------------------------------------
    struct Team {
        std::int32_t id = 0;
        std::string  name;
        std::string  league;
        std::string  record;
        std::string  logo;
        std::string  abbreviation;
        std::string  alias;
        std::string  createdAt;
        std::string  updatedAt;
        std::string  color;
        std::int32_t providerId = 0;
    };
    BOOST_DESCRIBE_STRUCT(Team, (),
        (id, name, league, record, logo, abbreviation, alias,
         createdAt, updatedAt, color, providerId))

    struct SportsMetadata {
        std::int32_t             id = 0;
        std::string              sport;
        std::string              image;
        std::string              resolution;
        std::string              ordering;
        std::vector<std::string> tags;          // server returns CSV string; parsed by glue
        std::string              series;
        std::string              createdAt;
    };
    BOOST_DESCRIBE_STRUCT(SportsMetadata, (),
        (id, sport, image, resolution, ordering, tags, series, createdAt))

    struct SportsMarketTypesResponse {
        std::vector<std::string> marketTypes;
    };
    BOOST_DESCRIBE_STRUCT(SportsMarketTypesResponse, (), (marketTypes))

    // ---------------------------------------------------------------------------
    // Related tags
    // ---------------------------------------------------------------------------
    struct RelatedTag {
        std::string  id;
        std::string  tagID;          // wire: "tagID"
        std::string  relatedTagID;   // wire: "relatedTagID"
        std::int32_t rank = 0;
    };
    BOOST_DESCRIBE_STRUCT(RelatedTag, (), (id, tagID, relatedTagID, rank))

    // ---------------------------------------------------------------------------
    // Comments
    // ---------------------------------------------------------------------------
    struct CommentPosition {
        std::string tokenId;
        double      positionSize = 0.0;
    };
    BOOST_DESCRIBE_STRUCT(CommentPosition, (), (tokenId, positionSize))

    struct CommentProfile {
        std::string                  name;
        std::string                  pseudonym;
        bool                         displayUsernamePublic = false;
        std::string                  bio;
        bool                         isMod = false;
        bool                         isCreator = false;
        std::string                  proxyWallet;
        std::string                  baseAddress;
        std::string                  profileImage;
        ImageOptimized               profileImageOptimized;
        std::vector<CommentPosition> positions;
    };
    BOOST_DESCRIBE_STRUCT(CommentProfile, (),
        (name, pseudonym, displayUsernamePublic, bio, isMod, isCreator,
         proxyWallet, baseAddress, profileImage, profileImageOptimized, positions))

    struct Reaction {
        std::string    id;
        std::int32_t   commentID = 0;
        std::string    reactionType;
        std::string    icon;
        std::string    userAddress;
        std::string    createdAt;
        CommentProfile profile;
    };
    BOOST_DESCRIBE_STRUCT(Reaction, (),
        (id, commentID, reactionType, icon, userAddress, createdAt, profile))

    struct Comment {
        std::string           id;
        std::string           body;
        std::string           parentEntityType;
        std::int32_t          parentEntityID = 0;
        std::string           parentCommentID;
        std::string           userAddress;
        std::string           replyAddress;
        std::string           createdAt;
        std::string           updatedAt;
        CommentProfile        profile;
        std::vector<Reaction> reactions;
        std::int32_t          reportCount = 0;
        std::int32_t          reactionCount = 0;
    };
    BOOST_DESCRIBE_STRUCT(Comment, (),
        (id, body, parentEntityType, parentEntityID, parentCommentID,
         userAddress, replyAddress, createdAt, updatedAt, profile,
         reactions, reportCount, reactionCount))

    // ---------------------------------------------------------------------------
    // Public profile
    // ---------------------------------------------------------------------------
    struct PublicProfileUser {
        std::string id;
        bool        creator = false;
        bool        isMod = false;       // wire: "mod"
    };
    BOOST_DESCRIBE_STRUCT(PublicProfileUser, (), (id, creator, isMod))

    struct PublicProfile {
        std::string                    createdAt;
        std::string                    proxyWallet;
        std::string                    profileImage;
        bool                           displayUsernamePublic = false;
        std::string                    bio;
        std::string                    pseudonym;
        std::string                    name;
        std::vector<PublicProfileUser> users;
        std::string                    xUsername;
        bool                           verifiedBadge = false;
    };
    BOOST_DESCRIBE_STRUCT(PublicProfile, (),
        (createdAt, proxyWallet, profileImage, displayUsernamePublic,
         bio, pseudonym, name, users, xUsername, verifiedBadge))

    // ---------------------------------------------------------------------------
    // Public search
    // ---------------------------------------------------------------------------
    struct SearchTag {
        std::string  id;
        std::string  label;
        std::string  slug;
        std::int32_t eventCount = 0;
    };
    BOOST_DESCRIBE_STRUCT(SearchTag, (), (id, label, slug, eventCount))

    struct Profile {
        std::string  id;
        std::string  name;
        std::int32_t user = 0;
        std::string  referral;
        std::int32_t createdBy = 0;
        std::int32_t updatedBy = 0;
        std::string  createdAt;
        std::string  updatedAt;
        std::string  utmSource;
        std::string  utmMedium;
        std::string  utmCampaign;
        std::string  utmContent;
        std::string  utmTerm;
        bool         walletActivated = false;
        std::string  pseudonym;
        bool         displayUsernamePublic = false;
        std::string  profileImage;
        std::string  bio;
        std::string  proxyWallet;
        ImageOptimized profileImageOptimized;
        bool         isCloseOnly = false;
        bool         isCertReq = false;
        std::string  certReqDate;
    };
    BOOST_DESCRIBE_STRUCT(Profile, (),
        (id, name, user, referral, createdBy, updatedBy, createdAt, updatedAt,
         utmSource, utmMedium, utmCampaign, utmContent, utmTerm,
         walletActivated, pseudonym, displayUsernamePublic, profileImage, bio,
         proxyWallet, profileImageOptimized, isCloseOnly, isCertReq, certReqDate))

    struct Pagination {
        std::int32_t hasMorePages = 0;     // shape varies; conservative default
        std::int32_t totalResults = 0;
    };
    BOOST_DESCRIBE_STRUCT(Pagination, (), (hasMorePages, totalResults))

    struct SearchResults {
        std::vector<Event>     events;
        std::vector<SearchTag> tags;
        std::vector<Profile>   profiles;
        Pagination             pagination;
    };
    BOOST_DESCRIBE_STRUCT(SearchResults, (), (events, tags, profiles, pagination))

    // ---------------------------------------------------------------------------
    // Request structs for the new endpoints
    // ---------------------------------------------------------------------------
    struct TagsRequest {
        std::optional<int>  limit;
        std::optional<int>  offset;
        std::string         order;
        std::optional<bool> ascending;
        std::optional<bool> include_template;
        std::optional<bool> is_carousel;
    };

    struct SeriesListRequest {
        std::optional<int>       limit;
        std::optional<int>       offset;
        std::string              order;
        std::optional<bool>      ascending;
        std::vector<std::string> slug;
        std::vector<std::string> categories_ids;
        std::vector<std::string> categories_labels;
        std::optional<bool>      closed;
        std::optional<bool>      include_chat;
        std::string              recurrence;
    };

    struct CommentsRequest {
        std::string         parent_entity_type;     // "Event" | "Series" | "market"
        std::string         parent_entity_id;
        std::optional<int>  limit;
        std::optional<int>  offset;
        std::string         order;
        std::optional<bool> ascending;
        std::optional<bool> get_positions;
        std::optional<bool> holders_only;
    };

    struct SearchRequest {
        std::string              q;                    // required
        std::optional<bool>      cache;
        std::string              events_status;
        std::optional<int>       limit_per_type;
        std::optional<int>       page;
        std::vector<std::string> events_tag;
        std::optional<int>       keep_closed_markets;
        std::string              sort;
        std::optional<bool>      ascending;
        std::optional<bool>      search_tags;
        std::optional<bool>      search_profiles;
        std::string              recurrence;
        std::vector<std::string> exclude_tag_id;
        std::optional<bool>      optimized;
    };

    struct TeamsRequest {
        std::optional<int>       limit;
        std::optional<int>       offset;
        std::string              league;
        std::vector<std::string> name;
    };

    // Used by GET /tags/{id}/related-tags?status=...
    struct RelatedTagsRequest {
        std::string status;        // "active" | "closed" | "all"
    };

}

#endif //POLYMARKET_GAMMA_TYPES_H
