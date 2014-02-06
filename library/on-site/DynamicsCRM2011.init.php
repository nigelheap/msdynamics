<?php

/* Include the class library for the Dynamics CRM 2011 Connector 
 *
 *  DynamicsCRM2011_Init::create()->set(
 *    $discoveryServiceURL, 
 *    $organizationUniqueName, 
 *    $loginUsername, 
 *    $loginPassword, 
 *    $debug = false
 *   );
 *  
 */
require_once 'DynamicsCRM2011.php';

/**
* Building a class that connects to 
*/
class DynamicsCRM2011_Init{
  
    /**
     * connction details
     */
    protected $discoveryServiceURL;
    protected $organizationUniqueName;
    protected $loginUsername;
    protected $loginPassword;
    protected $debug = false;

    /**
     * The connector storage
     */
    public $crmConnector = null;


    /**
     * Grab a copy of the class
     * @return [type] [description]
     */
    public static function create(){

      return new self();

    }

    /**
     * Set the connection details
     * @param string  $discoveryServiceURL    
     * @param string  $organizationUniqueName
     * @param string  $loginUsername          
     * @param string  $loginPassword          
     * @param boolean $debug      
     * @return class $this            
     */
    public function set($discoveryServiceURL = "", $organizationUniqueName = "", $loginUsername = "", $loginPassword = "", $debug = false){

      if(!empty($discoveryServiceURL)){
        $this->discoveryServiceURL = $discoveryServiceURL;
      }

      if(!empty($organizationUniqueName)){
        $this->organizationUniqueName = $organizationUniqueName;
      }

      if(!empty($loginUsername)){
        $this->loginUsername = $loginUsername;
      }

      if(!empty($loginPassword)){
        $this->loginPassword = $loginPassword;
      }

      if(!empty($debug)){
        $this->debug = $debug;
      }

      return $this;
       
    }

    /**
     * Run the connector and return 
     * @return class $this   
     */
    public function connect(){

      if(!is_null($this->crmConnector)){
        return $this;
      }

      $this->crmConnector = new DynamicsCRM2011_Connector(
        $this->discoveryServiceURL, 
        $this->organizationUniqueName, 
        $this->loginUsername, 
        $this->loginPassword, 
        $this->debug
      );

      return $this;

    }

    /**
     * find contact fields
     * @return [type] [description]
     */
    public function findContactFields(){


      $search = $this->searchQuery('contact');
      $search = $this->searchCRM($search);

      foreach ($search->Entities as $contact) {
        return $contact->properties;
        break;
      }

    }

    /**
     * Build search query
     * @param  string  $type   what to search for contact, account
     * @param  array   $fields fields to return
     * @param  integer $limit  how many do you want
     * @param  array   $search field search
     * @return string          built xml for seach
     */
    public function searchQuery($type, $fields = array(), $limit = 1, $search = array()){

        $queryFields = '<all-attributes/>';

        if(!empty($fields)){
            $queryFields = "";
            foreach($fields as $field){
                $queryFields .= '<attribute name="'.$field.'" />';
            }
        }
        
        $searchQuery = "";
        if(!empty($search)){
            $searchQuery .= '<filter type="and">';
                foreach($search as $key => $value){
                     $searchQuery .= '<condition attribute="'.$key.'" operator="eq" value="'.$value.'" />';
                }
            $searchQuery .= '</filter>';
          
        }

        return '<fetch version="1.0" output-format="xml-platform" mapping="logical" distinct="true" count="'.$limit.'">
                    <entity name="'.$type.'">
                            '.$queryFields.' 
                            '.$searchQuery.'
                        </entity>
                    </fetch>';

    }

    /**
     * Run the search
     * @param  string $accountQueryXML search xml from $this->searchQuery();  
     * @return object                  results
     */
    public function searchCRM($accountQueryXML){

       return $this->crmConnector->retrieveMultiple($accountQueryXML);

    }



    public function findAccounts($fields = array(), $limit = 1, $search = array()){

        $search = $this->searchQuery('account', $fields, $limit, $search);

        $accountData = $this->searchCRM($search);

        return $accountData->Entities;

        foreach ($accountData->Entities as $account) {

            //echo "<pre>".print_r($account, true)."</pre>";
            
            /* Fetch fields individually */
            echo "\t".'Account <'.$account->name.'> has ID {'.$account->accountid.'}'.PHP_EOL;
            /* Use specific toString of Contact */

            $contactId = $account->primarycontactid->id;
            echo "\t\t".'Primary Contact is: '.$account->PrimaryContactId.PHP_EOL;
            echo "\t\t".'Primary Contact id is : '.$account->primarycontactid->id.PHP_EOL;
            /* Use automatic toString of SystemUser */
            echo "\t\t".'Owner is: '.$account->OwnerId.PHP_EOL;
            
        }


    }

    public function findContacts($fields = array(), $limit = 1, $search = array()){

        $search = $this->searchQuery('contact', $fields, $limit, $search);

        $accountData = $this->searchCRM($search);

        return $accountData->Entities;

    }


    public function createUpdateContact($search, $fields, $username){

        //lets go and check if the contact exists
        $contactData = $this->findContacts(array(), 1, $search);

        $contactID = null;
        $type = 'create';

        if(!empty($contactData)){
          foreach ($contactData as $entry) {
            $contactID = $entry->ID;
            $type = 'update';
          }
        } 
        $contact = DynamicsCRM2011_Entity::fromLogicalName($this->crmConnector, 'contact');
        //$contact->firstname = $email;
        $contact->lastname = $username;
        if(!empty($contactID)){
          $contact->ID = $contactID;
        }

        foreach($fields as $key => $field){
          
          if($field['type'] == 'boolean'){
            $contact->$key = new DynamicsCRM2011_OptionSetValue($field['value'], (!empty($field['value']) ? 'Yes' : 'No'));
          } else {
            $contact->$key = $field['value'];
          }
          



        }
          
        
        $this->crmConnector->$type($contact);

        return $this;


    }



    public function deleteContact($search){

        //lets go and check if the contact exists
        $contactData = $this->findContacts(array(), 1, $search);

        if(!empty($contactData)){
          foreach ($contactData as $entry) {
            $this->crmConnector->delete($entry);
          }
        } 


    }

}





