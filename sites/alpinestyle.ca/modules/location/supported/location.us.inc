<?php
// $Id: location.us.inc,v 1.16.2.1 2007/03/12 22:08:07 ankur Exp $

/**
 * Returns a lat/lon pair of the approximate center of the given postal code in the given country
 *
 * @param $location
 *   An associative array $location where
 *     'street'       => the street portion of the location
 *     'supplemental' => additional street portion of the location
 *     'province'     => the province, state, or territory
 *     'country'      => lower-cased two-letter ISO code (REQUIRED)
 *     'postal_code'  => the international postal code for this location (REQUIRED)
 *
 * @return
 *   An associative array where
 *      'lat' => approximate latitude of the center of the postal code's area
 *      'lon' => approximate longitude of the center of the postal code's area
 *
 */
function location_latlon_rough_us($location = array()) {
  if (!isset($location['postal_code'])) {
    return NULL;
  }

  $result = db_query("SELECT latitude, longitude FROM {zipcodes} WHERE country = '%s' AND zip = '%s'", $location['country'], substr(str_pad($location['postal_code'], 5, '0', STR_PAD_LEFT), 0, 5));
  
  if ($row = db_fetch_object($result)) {
    return array('lat' => $row->latitude, 'lon' => $row->longitude);
  }
  else {
    return NULL;
  }
}

/**
 * Returns a lat/lon pair of the approximate center of the given postal code in the given country
 *
 * @param $location
 *   An associative array $location where only postal code and country are necessary, but can have the keys:
 *     'street'       => the street portion of the location
 *     'supplemental' => additional street portion of the location
 *     'province'     => the province, state, or territory
 *     'country'      => lower-cased two-letter ISO code (REQUIRED)
 *     'postal_code'  => the international postal code for this location (REQUIRED)
 *
 * @return
 *   An associative array where
 *      'lat' => approximate latitude of the center of the postal code's area
 *      'lon' => approximate longitude of the center of the postal code's area
 *
 */
function location_get_postalcode_data_us($location = array()) {
  $dash_index = strpos($location['postal_code'], '-');
  // First we strip slash off if we're dealing with a 9-digit US zipcode
  if (!($dash_index === FALSE)) {
    $location['postal_code'] = substr($location['postal_code'], 0, $dash_index);
  }
  
  // Now we pad the thing and query.
  $res = db_query("SELECT * FROM {zipcodes} where country = '%s' AND zip = '%s'", $location['country'], str_pad($location['postal_code'], 5, "0", STR_PAD_LEFT));
  if ($row = db_fetch_object($res)) {
    return array('lat' => $row->latitude, 'lon' => $row->longitude, 'city' => $row->city, 'province' => $row->state, 'country' => $row->country);
  }
  else {
    return NULL;
  } 
}

/**
 * Parameters:
 *   An associative array $location where
 *     'street'       => the street portion of the location
 *     'supplemental' => additional street portion of the location
 *     'province'     => the province, state, or territory
 *     'country'      => lower-cased two-letter ISO code (REQUIRED)
 *     'postal_code'  => the international postal code for this location (REQUIRED)
 *
 *
 */
function location_latlon_exact_us($location = array()) {
  return NULL;
  // By uncommenting the line of code below, you legally acknowledge that you understand that you can only
  // do so under the terms of the Non-Commercial Share-alike license described at http://creativecommons.org/licenses/by-nc-sa/2.0/
  //return _location_latlon_exact_us_geocoder($location);
}


/**
 * Calls up a web-service to retrieve a lat/lon pair for a full, correct U.S. location.
 *
 * @param $location
 *    An associative array that represents an location where
 *        'street'       => is the street location
 *        'supplemental' => any supplemental portion to the street location
 *        'city'         => city name
 *        'province'     => state, province, or territorial abbreviation
 *        'postal_code'  => postal code
 *
 * @return
 *    An associative array where
 *        'lat' => Is a float value in latitude
 *        'lon' => Is a float value in longitude 
 *    If the location supplied does not provide enough information, NULL is returned.
 *    "Enough information" means that there is either 
 *    (a valid 'street' AND valid 'postal_code') OR (valid 'street' AND valid 'city' AND valid 'province')
 */
function _location_latlon_exact_us_geocoder($location = array()) {
  $location_string = '';
  if (isset($location['street']) && trim($location['street']) != '') {
    if (isset($location['postal_code'])) {
      $location_string = $location['street'] .' '. $location['postal_code'];
    }
    elseif (isset($location['city']) && isset($location['province']) && trim($location['city']) != '' && trim($location['province'])) {
      $location_string = $location['street'] .', '. $location['city'] .', '. $location['province'];
    }
    else { // else geocoder.us won't do bidness with you!
      return NULL;
    }
  }
  else {
    return NULL;
  }

  $result = xmlrpc('http://rpc.geocoder.us/service/xmlrpc', 'geocode', $location_string);
  
  if (is_array($result) && is_array($result[0]) && isset($result[0]['lat']) && is_numeric($result[0]['lat']) && isset($result[0]['long']) && is_numeric($result[0]['long'])) {
    return array('lat' => $result[0]['lat'], 'lon' => $result[0]['long']);
  }
  return NULL;
}


function location_map_link_us_yahoo($location = array()) {
  $get_query = '?';
  if (isset($location['street'])) {
    $get_query .= 'addr='. urlencode($location['street']) .'&amp;';
  }
  if ($location['province'] != '' || $location['city'] != '' || $location['postal_code'] != '') {
    $get_query .= 'csz='. _location_us_yahoo_csz_get_field($location) .'&amp;';
  }
  $get_query .= 'country='. urlencode($location['country']);
  return ('http://maps.yahoo.com/maps_result'. $get_query);
}


function location_map_link_us_google($location = array()) {
  $query_params = array();
  
  foreach (array('street', 'city', 'province', 'postal_code', 'country') as $field) {
    if (isset($location[$field])) {
      $query_params[] = $location[$field];
    }
  }
  
  if (count($query_params)) {
    return ('http://maps.google.com?q='. urlencode(implode(", ", $query_params)));
  }
  else {
    return NULL;
  }
}

function location_map_link_us_mapquest($location = array()) {
  if (isset($location['street'])) {
    $get_query .= 'address='. urlencode($location['street']) .'&amp;';
  }
  
  if (isset($location['city'])) {
    $get_query .= 'city='. urlencode($location['city']) .'&amp;';
  }
  
  if (isset($location['province'])) {
    $get_query .= 'state='. urlencode($location['province']) .'&amp;';
  }
  
  if (isset($location['postal_code'])) {
    $get_query .= 'zipcode='. urlencode($location['postal_code']);
  }
  if (strlen($get_query)) {
    return 'http://www.mapquest.com/maps/map.adp?searchtype=address&amp;country=US&amp;'. $get_query;
  }
  else {
    return NULL;
  }
}

/**
 * @return 
 *   An array where 
 *     -> the key is the word that helps identify the name of function that builds the link.  For example, a key of 'yahoo' means the name of the
 *        the function that builds a link to a map on Yahoo! Maps would be 'location_map_link_us_yahoo'
 *     -> the value is itself an array with 3 key/value pairs:
 *          'name' => points to the name of the mapping service.  For 'yahoo', this would be 'Yahoo! Maps'
 *          'url' => the url of the main page of the mapping service.  For 'yahoo', this would be 'http://maps.yahoo.com'
 *          'tos' => the url of the page that explains the map providers Terms of Service, or Terms of Use. For 'yahoo', this would be
 *                   'http://help.yahoo.com/help/us/maps/maps-24.html'
 */
function location_map_link_us_providers() {
  return array('google' => array('name' => 'Google Maps', 'url' => 'http://maps.google.com', 'tos' => 'http://www.google.com/help/terms_local.html'),
               'yahoo' => array('name' => 'Yahoo! Maps', 'url' => 'http://maps.yahoo.com', 'tos' => 'http://help.yahoo.com/help/us/maps/maps-24.html'),
               'mapquest' => array('name' => 'MapQuest', 'url' => 'http://www.mapquest.com', 'tos' => 'http://www.mapquest.com/features/main.adp?page=legal')
              );
}


/**
 * @return
 *   An array of values that work as keys to the array returned by location_map_link_us_providers.  The idea is that if the
 *   administrator of the site has not yet had a chance to visit the "Map Links" subtab on the location module's settings page,
 *   that we can provide deep-linking to a relatively safe default.  By 'relatively safe', we mean that the Terms Of Service of
 *   the provider of the maps are flexible enough for most parties.
 *
 *   For the case of the U.S., 'google' has relatively flexible Terms Of Service, whereas Yahoo! Maps and MapQuest have more
 *   restrictive Terms Of Service.
 *
 */
function location_map_link_us_default_providers() {
  return array('google');
}

function location_geocode_us_providers() {
  return array(
    'yahoo' => array('name' => 'Yahoo! Maps Web Services', 'url' => 'http://developer.yahoo.com/maps/rest/V1/geocode.html', 'tos' => 'http://developer.yahoo.com/maps/mapsTerms.html'),
    
    'google' => array('name' => 'Google Maps API', 'url' => 'http://www.google.com/apis/maps/', 'tos' => 'http://www.google.com/apis/maps/terms.html')
  
  );
}

function location_geocode_us_yahoo_settings(){
  $form = array();

  $form['location_geocode_us_yahoo_appid'] = array(
    '#type' => 'textfield',
    '#title' => t('Yahoo! Web Services Application ID'),
    '#size' => 64,
    '#maxlength' => 128,
    '#default_value' => variable_get('location_geocode_us_yahoo_appid', 'YahooDemo'),
    '#description' => t('Unless you are using this site to test and develop, you will need to obtain a Yahoo! Web Services Application ID from the %network_link.  If you are using for development and testing purposes, you can use \'YahooDemo\' as your AppID.  When getting an Application ID from Yahoo!, please also be sure to review the %usage_policy.', array('%network_link' => '<a href="http://api.search.yahoo.com/webservices/register_application">Yahoo! Developer Network</a>', '%usage_policy' => '<a href="http://developer.yahoo.com/usagePolicy/index.html">usage policy</a>'))
  );

  return $form;
}



function location_geocode_us_yahoo($location = array()) {
  $service_url = "http://api.local.yahoo.com/MapsService/V1/geocode?appid=". variable_get('location_geocode_us_yahoo_appid', "YahooDemo") ."&location=";
  
  $address = location_address2singleline($location);
  
  $http_reply = drupal_http_request($service_url . urlencode($address));
  
  // address may have been improperly formatted or invalid
  if ($http_reply->code == 400) {
    return NULL;
  }
  else {
    // got a successful reply, but we only want to return if we have address-level precision
    $matches = array();
    preg_match('/precision="([a-z]*)"/', $http_reply->data, $matches);

    if ($matches[1] != 'address') {
      // The precision we got back was not down to the street-address level
      return NULL;
    }
    else {
      $lat_match = array();
      $lon_match = array();
      $latlon = array();
      
      if (preg_match('/<Latitude>(.*)<\/Latitude>/', $http_reply->data, $lat_match)) {
        $latlon['lat'] = $lat_match[1];
      }
      else {
        return NULL;
      }
      
      
      if (preg_match('/<Longitude>(.*)<\/Longitude>/', $http_reply->data, $lon_match)) {
        $latlon['lon'] = $lon_match[1];

        return $latlon;
      }
      else {
        return NULL;
      }
    }
  }
}

function location_geocode_us_google($location = array()) {
  $service_url = 'http://maps.google.com/maps/geo?output=xml&key='. variable_get('location_geocode_us_google_apikey', '') .'&q=';

  $address = location_address2singleline($location);
  
  $http_reply = drupal_http_request($service_url . urlencode($address));
  
  $status_code_match = array();
  preg_match('/<code>(.*)<\/code>/', $http_reply->data, $status_code_match);
  $status_code = $status_code_match[1];
  if ($status_code != 200) {
    return NULL;
  }
  
  $accuracy_code_match = array();
  preg_match('/Accuracy="([0-9])"/', $http_reply->data, $accuracy_code_match);
  $accuracy_code = $accuracy_code_match[1];
  if ($accuracy_code != 8 && $accuracy_code != 7) {
    return NULL;
  }
  
  $latlon_match = array();
  preg_match('/<coordinates>(.*)<\/coordinates>/', $http_reply->data, $latlon_match);
  
  $latlon_exploded = explode(',', $latlon_match[1]);
  
  return array('lat' => $latlon_exploded[1], 'lon' => $latlon_exploded[0]);
}

function location_geocode_us_google_settings(){
  $form = array();

  $form['location_geocode_us_google_apikey'] = array(
    '#type' => 'textfield',
    '#title' => t('Google Maps API Key'),
    '#size' => 64,
    '#maxlength' => 128,
    '#default_value' => variable_get('location_geocode_us_google_apikey', ''),
    '#description' => t('In order to use the Google Maps API geocoding web-service, you will need a Google Maps API Key.  You can obtain one at the !sign_up_link for the !google_maps_api.', array('!sign_up_link' => '<a href="http://www.google.com/apis/maps/signup.html">sign-up page</a>', '!google_maps_api' => '<a href="http://www.google.com/apis/maps/">Google Maps API</a>'))
  );

  return $form;
}

/**
 * Parameters:
 *   -> $locationA is an associative array that represents a full location where
 *        'street'       => the street portions of the location
 *        'supplemental' => additional street portion of the location
 *        'province'     => the province, state, or territory
 *        'country'      => lower-cased two-letter ISO code (REQUIRED)
 *   -> $locationB is associative array that represents a full location in the same way that
 *       parameter $locationB does.
 *
 * Returns: a link to driving directions
 *
 * For now, assume site-admin wants American driving directions linked to Yahoo! Driving Directions.
 * Maybe later, we can add some kind of country-specific settings page that allows the site-admin to
 * decide which site to link to for driving directions.
 */
function location_driving_directions_link_us($locationA, $locationB) {
  return _location_driving_directions_link_us_yahoo($locationA, $locationB);
}

/**
 * Parameters:
 *    Function that is called by location_driving_directions_link_us() under assumption that it
 *    is the chosen function
 *
 * Returns:
 *    a URL with HTTP GET variables
 *    Depending on how full the locationes are, the URL will either point to the driving directions
 *    on Yahoo! or, if only partial locationes are provided, a URL that points to the *form* for
 *    Yahoo! driving directions where the form is filled with whatever fields have been provided
 *    for the partial location(es).  
 */
function _location_driving_directions_link_us_yahoo($locationA, $locationB) {
  if (trim($locationB['country']) != 'ca' && trim($locationB['country']) != 'us') {
    return '';
  }
  
  // These are the fields that need to be in each location if we are to provide a direct
  // link to yahoo directions.  If all of these fields don't have values, then we generate
  // a link to the *form* for Yahoo! driving directions rather than directly to the driving
  // directions themselves.
   
  
  foreach ($locationA as $field => $value) {
    $locationA[$field] = trim($value);
  }
  
  foreach ($locationB as $field => $value) {
    $locationB[$field] = trim($value);
  }
  
  if (_location_us_enough_fields_for_yahoo($locationA) && _location_us_enough_fields_for_yahoo($locationB)) {
    $yahoo_maps_path = 'dd_result';
  }
  else {
    $yahoo_maps_path = 'dd';
  }
  
  $get_query  = '?';
  
  $get_query .= 'addr='. urlencode($locationA['street']) .'&amp;';
  $get_query .= 'csz=' . _location_us_yahoo_csz_get_field($locationA) .'&amp;';
  $get_query .= 'country='. urlencode($locationA['country']) .'&amp;';
  
  $get_query .= 'taddr='. urlencode($locationB['street']) .'&amp;';
  $get_query .= 'tcsz=' . _location_us_yahoo_csz_get_field($locationB) .'&amp;';
  $get_query .= 'tcountry='. urlencode($locationB['country']);
  
  $get_query .= '&amp;getrte='. urlencode('Get Directions');
  
  return ('http://maps.yahoo.com/'. $yahoo_maps_path . $get_query);
}


function _location_us_enough_fields_for_yahoo($location) {
  // These are the fields that need to be in each location if we are to provide a direct
  // link to yahoo directions.  If all of these fields don't have values, then we generate
  // a link to the *form* for Yahoo! driving directions rather than directly to the driving
  // directions themselves.
  if (strlen($location['street']) && strlen($location['city']) && strlen($location['province'])) {
    return TRUE;
  }
  
  if (strlen($location['street']) && strlen($location['postal_code'])) {
    return TRUE;
  }
  
  if (strlen($location['street']) && strlen($location['city']) && strlen($location['province'])) {
    return TRUE;
  }
  
  return FALSE;
}

// Don't mess with this function unless you understand its logic.  It has to do with
// the question of "to comma or not to comma?"
function _location_us_yahoo_csz_get_field($location) {
  // For some reasons, to the end of pinpointing a location, Yahoo! Maps and Driving Directions
  // do better a better job with retrieving info based strictly on a Canadian city/province
  // than on a Canadian postal code.
  if ($location['country'] = 'ca') {
    if (strlen($location['city']) && strlen($location['province'])) {
      return urlencode($location['city'] .', '. $location['province']);
    }
    
    if (strlen($location['postal_code'])) {
      return urlencode($location['postal_code']);
    }
  }
  else {
    if (strlen($location['postal_code'])) {
      return urlencode($location['postal_code']);
    }
  
    if (strlen($location['city']) && strlen($location['province'])) {
      return urlencode($location['city'] .', '. $location['province']);
    }
  }
  
  if (strlen($location['city']) || strlen($location['state'])) {
    if (strlen($location['city'])) {
      return urlencode($location['city']);
    }
    else {
      return urlencode($location['state']);
    }
  }
  return '';
}

function _location_us_geocoder_oneline($location = array()) {
  $line = '';
  
  $line .= $location['street'] . ', ';
  
  if (strlen($location['city']) && strlen($location['province']) && strlen($location['postal_code'])) {
    $line .= $location['city'] .', '. $location['province'] .' '. $location['postal_code'];
  } 
  elseif (strlen($location['city']) && strlen($location['province'])) {
    $line .= $location['city'] .', '. $location['state'];
  }
  elseif (strlen($location['postal_code'])) {
    if (strlen($location['city']) || strlen($location['state'])) {
      if (strlen($location['city'])) {
        $line .= $location['city'] .', '. $location['postal_code'];
      }
      else {
        $line .= $location['state'] .', '. $location['postal_code'];
      }
    }
    else {
      $line .= $location['postal_code'];
    }
  }
  // DEBUG: commented code is for testing/debugging purposes
  //print '_location_us_geocoder_oneline() RETURNING '. $line ."<br/>\n";
  return $line;
}



/**
 * Returns an associative array of states/territories where
 *   -> the keys are integers starting from 1
 *   -> the values are the English names for those states/territories
 *
 * The states are grouped together at the beginning of the array and sorted
 * alphabetically.
 *
 * The territories are grouped together at the end of the array and sorted
 * alphabetically.
 *
 */
function location_province_list_us() {
  return array('AL' => 'Alabama',
               'AK' => 'Alaska',
               'AZ' => 'Arizona',
               'AR' => 'Arkansas',
               'CA' => 'California',
               'CO' => 'Colorado',
               'CT' => 'Connecticut',
               'DE' => 'Delaware',
               'DC' => 'District Of Columbia',
               'FL' => 'Florida',
               'GA' => 'Georgia',
               'HI' => 'Hawaii',
               'ID' => 'Idaho',
               'IL' => 'Illinois',
               'IN' => 'Indiana',
               'IA' => 'Iowa',
               'KS' => 'Kansas',
               'KY' => 'Kentucky',
               'LA' => 'Louisiana',
               'ME' => 'Maine',
               'MD' => 'Maryland',
               'MA' => 'Massachusetts',
               'MI' => 'Michigan',
               'MN' => 'Minnesota',
               'MS' => 'Mississippi',
               'MO' => 'Missouri',
               'MT' => 'Montana',
               'NE' => 'Nebraska',
               'NV' => 'Nevada',
               'NH' => 'New Hampshire',
               'NJ' => 'New Jersey',
               'NM' => 'New Mexico',
               'NY' => 'New York',
               'NC' => 'North Carolina',
               'ND' => 'North Dakota',
               'OH' => 'Ohio',
               'OK' => 'Oklahoma',
               'OR' => 'Oregon',
               'PA' => 'Pennsylvania',
               'RI' => 'Rhode Island',
               'SC' => 'South Carolina',
               'SD' => 'South Dakota',
               'TN' => 'Tennessee',
               'TX' => 'Texas',
               'UT' => 'Utah',
               'VT' => 'Vermont',
               'VA' => 'Virginia',
               'WA' => 'Washington',
               'WV' => 'West Virginia',
               'WI' => 'Wisconsin',
               'WY' => 'Wyoming',
               
               'AS' => 'American Samoa',
               'FM' => 'Federated States of Micronesia',
               'GU' => 'Guam',
               'MH' => 'Marshall Islands',
               'MP' => 'Northern Mariana Islands',
               'PW' => 'Palau',
               'PR' => 'Puerto Rico',
               'VI' => 'Virgin Islands'
              );
}


/**
 * Returns an associative array of states/territories where
 *   -> the keys are integers starting from 1
 *   -> the values are the English names for those states/territories
 *
 * The states are grouped together at the beginning of the array and sorted
 * alphabetically.
 *
 * The territories are grouped together at the end of the array and sorted
 * alphabetically.
 *
 * Currently not being used, but may be in order to be compatible with CiviCRM
 * TODO: These numeric indices need to be changed to match those in the CiviCRM database
 */
function location_province_list_numeric_us() {
  return array('001' => 'Alabama',
               '002' => 'Alaska',
               '003' => 'Arizona',
               '004' => 'Arkansas',
               '005' => 'California',
               '006' => 'Colorado',
               '007' => 'Connecticut',
               '008' => 'Delaware',
               '009' => 'District Of Columbia',
               '010' => 'Florida',
               '011' => 'Georgia',
               '012' => 'Hawaii',
               '013' => 'Idaho',
               '014' => 'Illinois',
               '015' => 'Indiana',
               '016' => 'Iowa',
               '017' => 'Kansas',
               '018' => 'Kentucky',
               '019' => 'Louisiana',
               '020' => 'Maine',
               '021' => 'Maryland',
               '022' => 'Massachusetts',
               '023' => 'Michigan',
               '024' => 'Minnesota',
               '025' => 'Mississippi',
               '026' => 'Missouri',
               '027' => 'Montana',
               '028' => 'Nebraska',
               '029' => 'Nevada',
               '030' => 'New Hampshire',
               '031' => 'New Jersey',
               '032' => 'New Mexico',
               '033' => 'New York',
               '034' => 'North Carolina',
               '035' => 'North Dakota',
               '036' => 'Ohio',
               '037' => 'Oklahoma',
               '038' => 'Oregon',
               '039' => 'Pennsylvania',
               '040' => 'Rhode Island',
               '041' => 'South Carolina',
               '042' => 'South Dakota',
               '043' => 'Tennessee',
               '044' => 'Texas',
               '045' => 'Utah',
               '046' => 'Vermont',
               '047' => 'Virginia',
               '048' => 'Washington',
               '049' => 'West Virginia',
               '050' => 'Wisconsin',
               '051' => 'Wyoming',
               
               '052' => 'American Samoa',
               '053' => 'Federated States of Micronesia',
               '054' => 'Guam',
               '055' => 'Marshall Islands',
               '056' => 'Northern Mariana Islands',
               '057' => 'Palau',
               '058' => 'Puerto Rico',
               '059' => 'Virgin Islands'
              );
}

?>
