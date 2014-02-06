<?php

/**
 * This file holds the drupal functions the run the dynamics connector
 */

function msdynamics_config(){

    $discoveryServiceURL = variable_get('msdynamics_organizationdiscoveryurl', '');
    $organizationUniqueName = variable_get('msdynamics_organizationuniquename', '');
    $loginUsername = variable_get('msdynamics_loginusername', '');
    $loginPassword = variable_get('msdynamics_loginpassword', '');

    $debug = false;

    return DynamicsCRM2011_Init::create()->set($discoveryServiceURL, $organizationUniqueName, $loginUsername, $loginPassword, $debug = false);

}


function msdynamics_testConnection(){

    $crm = msdynamics_config();

    $result = "";
    $result .= 'Connect started at '.date('Y-m-d H:i:s') . PHP_EOL;
    $result .= "Connecting to the CRM... " . PHP_EOL;
    $crm->connect();
    $result .= 'Connected at '.date('Y-m-d H:i:s').PHP_EOL;

    return $result;

}

function msdynamics_indexContactFields(){

    $crm = msdynamics_config();
    $crm->connect();

    $properties = $crm->findContactFields();

    $currentRecords = array();
    $insertValues = array();
    $updateValues = array();

    $insertCount = 0;
    $updateCount = 0;
    $deleteCount = 0;

    $return = "";

    $result = db_query('SELECT * FROM {msdynamics}');

    foreach ($result as $record) {
      $currentRecords[] = $record->msdynamics_fieldname;
    }
    
    foreach($properties as $key => $value){
        if(in_array($key, $currentRecords)){
            $updateValues[] = array(
                'msdynamics_fieldname' => $key,
                'msdynamics_fieldlabel' => $value['Label'],
                'msdynamics_fielddescription' => $value['Description'],
                'msdynamics_fieldrequired' => $value['RequiredLevel'],
            ); 
        } else {
            $insertValues[] = array(
                'msdynamics_fieldname' => $key,
                'msdynamics_fieldlabel' => $value['Label'],
                'msdynamics_fielddescription' => $value['Description'],
                'msdynamics_fieldrequired' => $value['RequiredLevel'],
            );  
        }

        $currentRecords = array_diff($currentRecords, array($key));
    }

    /**
     * update new values
     */
    if(!empty($updateValues)){


        foreach ($updateValues as $record) {
          $num_updated = db_update('msdynamics')
              ->fields($record)
              ->condition('msdynamics_fieldname', $record['msdynamics_fieldname'], '=')
              ->execute();

          
           $updateCount++;

        }

    }

    /**
     * insert new values
     */
    if(!empty($insertValues)){

        $query = db_insert('msdynamics')->fields(array(
            'msdynamics_fieldname', 
            'msdynamics_fieldlabel', 
            'msdynamics_fielddescription',
            'msdynamics_fieldrequired',
        ));

        foreach ($insertValues as $record) {
          $query->values($record);
          $insertCount++;
        }
        $query->execute();
    }


    if(false && !empty($currentRecords)){
        foreach($currentRecords as $value){

            $deleteCount++;

            db_delete('msdynamics') 
              ->condition('msdynamics_fieldname', $value)
              ->execute();

            
        }
    }

    $return .= "Insert Count: " . $insertCount . PHP_EOL;
    $return .= "Updated Count: " . $updateCount . PHP_EOL;
    $return .= "Deleted Count: " . count($currentRecords) . PHP_EOL;

    $return .= "Deleted Values " . PHP_EOL . print_r($currentRecords, true). PHP_EOL;
    $return .= "Updated Values " . PHP_EOL . print_r($updateValues, true). PHP_EOL;
    $return .= "Create Values " . PHP_EOL . print_r($insertValues, true). PHP_EOL;

    variable_set('msdynamics_indexed', 1);

    return $return;
}


function msdynamics_findAccounts(){

      $fields = array(
              'name',
              'address1_city',
              'telephone1',
              'accountid',
      );

      $fields = array();

      $accountId = 'da1887f9-3d73-e311-aef6-005056ba549a';
      array('ownerid' => $accountId);

}

function msdynamics_findContacts(){

      $fields = array(
              'name',
              'address1_city',
              'telephone1',
              'accountid',
      );

      $fields = array();

      $accountId = 'da1887f9-3d73-e311-aef6-005056ba549a';
      array('ownerid' => $accountId);
}


function msdynamics_updateCreateContact($account){

    $dyanmicsFields = msdynamics_load_fields(true);
    $fields = array();
    $search = array();

    $primaryKey = variable_get('msdynamics_primary_key', 'mail');

    foreach($dyanmicsFields as $dyanmicsField){

        $userField = $dyanmicsField['user_fieldname'];

        if(!isset($account->$userField)){
            continue;
        }

        $type = $dyanmicsField['user_fieldtype'];
        $crmField = $dyanmicsField['msdynamics_fieldname'];
        $fields[$crmField]['type'] = $type;

        if($type == "boolean"){
            $fields[$crmField]['value'] = '0';
        }

        if((is_string($account->$userField) || is_numeric($account->$userField)) && 
            in_array($type, array('string', 'boolean')) && 
            !empty($account->$userField)){

            $fields[$crmField]['value'] = $account->$userField;
            

        } elseif(is_array($account->$userField) && 
            in_array($type, array('string', 'boolean')) && 
            $item = field_get_items('user', $account, $userField)){

            $fields[$crmField]['value'] = $item[0]['value'];

        }

        if($primaryKey == $userField){
            $search[$crmField] = $fields[$crmField]['value'];
        }
        
    }

    //dpm($search);

    if(empty($search)){
        return;
    }

    //dpm($account);
    //dpm($fields);

    $crm = msdynamics_config();
    $crm->connect();
    $crm->createUpdateContact($search, $fields, $account->name);


}


function msdynamics_deleteContact($account){

    $primaryKey = variable_get('msdynamics_primary_key', 'mail');
    $dyanmicsFields = msdynamics_load_fields(true);
    $search = array();

    foreach($dyanmicsFields as $dyanmicsField){

        $userField = $dyanmicsField['user_fieldname'];

        if(!isset($account->$userField)){
            continue;
        }

        $crmField = $dyanmicsField['msdynamics_fieldname'];

        if($primaryKey == $userField){
            $search[$crmField] = $account->$userField;
        }

    }

    $crm = msdynamics_config();
    $crm->connect();
    $crm->deleteContact($search);


}


/**
 * 
 */












