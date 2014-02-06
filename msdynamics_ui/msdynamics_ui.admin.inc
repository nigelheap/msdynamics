<?php

/**
 * PAGE CALLBACKS
 */

/**
 * Page callback for condig page
 * @return page vars ready for theming
 */
function msdynamics_ui_config(){

  $run = $tester_result = $index_result = "";

  if (!empty($_REQUEST['run'])) {
    $run = trim($_REQUEST['run']);
  }

  switch ($run){
    case 'test': 
        $tester_result = msdynamics_testConnection();
    break;
    case 'index':
        $index_result = msdynamics_indexContactFields();
    break;
  }


  $build = theme('msdynamics_config', array(
    'tester_result' => $tester_result,
    'index_result' => $index_result,
    'form' => drupal_get_form('msdynamics_ui_config_form'),
    'tester' => drupal_get_form('msdynamics_ui_test_form'),
    'index' => drupal_get_form('msdynamics_ui_index_form'),
  ));

  return $build;

}


function msdynamics_ui_mapping(){

  $build = theme('msdynamics_mapping', array(
    'form' => drupal_get_form('msdynamics_ui_mapping_form'),
  ));

  return $build;

}


/**
 * FORMS
 */

function msdynamics_ui_mapping_form(){

    $dynamicsfields = msdynamics_load_fields();
    $fields = msdynamics_ui_load_user_fields();

    $fieldtypes = msdynamics_get_fieldtypes();

    $options = array('' => t('-- Please Select --'));
    $userfieldvalues = array();
    $userfieldvalidation = array();
    $rows = array();

    foreach($fields as $field){
        $options[$field['Field']] = t($field['Field']);

        $userfieldvalues[] = $field['Field'];
        $userfieldvalidation[$field['Field']] = $field['Field'];
    }


    foreach($dynamicsfields as $key => $field){

        //dpm($field['msdynamics_fieldname']);
        //dpm((!empty($field['user_fieldname']) ? $field['user_fieldname'] : ''));

        $rows[] = array(
            array('data' => 
                array(
                    '#markup' => $field['msdynamics_fieldname'],
                )
            ),
            array('data' => 
                array(
                    '#markup' => $field['msdynamics_fieldlabel'],
                )
            ),
            array('data' => 
                array(
                    '#type' => 'select',
                    '#name' => 'dynamics[' . $field['msdynamics_fieldname'] . ']',
                    '#value' => (!empty($field['user_fieldname']) ? $field['user_fieldname'] : ''), 
                    '#options' => $options
                )
            ),
            array('data' => 
                array(
                    '#type' => 'select',
                    '#name' => 'typeof[' . $field['msdynamics_fieldname'] . ']',
                    '#value' => (!empty($field['user_fieldtype']) ? $field['user_fieldtype'] : ''), 
                    '#options' => $fieldtypes
                )
            ),
            array('data' => array(
                    '#name' => 'callback[' . $field['msdynamics_fieldname'] . ']',
                    '#type' => 'textfield',
                    '#value' => (!empty($field['user_fieldcallback']) ? $field['user_fieldcallback'] : ''),
                )
            ),
        );


    }

    $form['mapping']['primary_key'] = array(
        '#type' => 'select',
        '#options' => $options,
        '#value' => variable_get('msdynamics_primary_key', 'mail'),
        '#title' => t('Primary key for searching and linking to Dynamics'),
    );

    $form['mapping']['table'] = array(
      '#prefix' => '<div id="mapping">',
      '#suffix' => '</div>',
      '#tree' => TRUE,
      '#theme' => 'table',
      '#title' => t('Field Mapping'),
      '#header' => array(
            
            'dynamicsfield' => t('Dynamics Field'), 
            'dynamicsfieldlabel' => t('Dynamics Field Label'), 
            'userfield' => t('User field'), 
            'typeoffield' => t('Type'),
            'callback' => t('Callback Function (only active with callback type)')
        ),
      '#rows' => $rows,
    );


    $form['mapping']['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Save mapping'),
    );

    $form['#submit'][] = 'msdynamics_ui_mapping_submit';

    return $form;    

}



/**
 * Run a connection test
 * @return [type] [description]
 */
function msdynamics_ui_test_form(){

  $organizationuniquename = variable_get('msdynamics_organizationdiscoveryurl', '');
  $organizationuniquename = empty($organizationuniquename);

  $form['test'] = array(
    '#type' => 'fieldset',
    '#title' => t('Test Details'),
  );

  $form['test']['run'] = array(
    '#type' => 'hidden',
    '#value' => 'test',
    '#title' => t('Test Details'),
  );

  if($organizationuniquename){
    $form['test']['message']['#markup'] = msdynamics_ui_notavailable();
  }

  $form['test']['actions']['submit'] = array(
    '#type' => 'submit',
    '#disabled' => $organizationuniquename,
    '#value' => t('Test configuration'),
  );

  $form['#submit'][] = 'msdynamics_ui_config_indextest_submit';

  return $form;

}


function msdynamics_ui_index_form(){

  $organizationuniquename = variable_get('msdynamics_organizationdiscoveryurl', '');
  $organizationuniquename = empty($organizationuniquename);

  // Administration theme settings.
  $form['index'] = array(
    '#type' => 'fieldset',
    '#title' => t('Index'),
  );

  $form['test']['run'] = array(
    '#type' => 'hidden',
    '#value' => 'index',
    '#title' => t('Test Details'),
  );

  if($organizationuniquename){
    $form['index']['message']['#markup'] = msdynamics_ui_notavailable();
  }

  

  $form['index']['info']['#markup'] = '<div class="messages warning"><ul>'.
  '<li>You need at lease one contact in the Microsoft Dynamics to run this</li>' .
  '<li>This indexing needs to be done for the module to work</li>' .
  '</ul></div>';


  $form['index']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Index customer fields from Microsoft Dynamics'),
    '#disabled' => $organizationuniquename,
  );

  $form['#submit'][] = 'msdynamics_ui_config_indextest_submit';

  return $form;

}

/**
 * Form call back for config page
 * @return form drupal form ready to build
 */
function msdynamics_ui_config_form(){


  $form['#action'] = "admin/config/msdynamics/connection";
  // Administration theme settings.
  $form['connection'] = array(
    '#type' => 'fieldset',
    '#title' => t('Connection Details'),
  );

  $vars = msdynamics_get_vars();

  foreach($vars as $key => $var){

      $form['connection'][$key] = array(
        '#type' => 'textfield',
        '#title' => t($var['title']),
        '#description' => t($var['description']),
        '#default_value' => variable_get('msdynamics_'.$key, ''),
        '#size' => 60,
        '#required' => TRUE,
      );

  }  

  $form['connection']['actions'] = array('#type' => 'actions');
  $form['connection']['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
  );


  $form['#submit'][] = 'msdynamics_ui_config_submit';

  return $form;


}


/**
 * SUBMIT HANDLERS
 */

function msdynamics_ui_mapping_submit($form, &$form_state){
    $post = !empty($_POST) ? $_POST : null;
    
    $num_updated = 0;
    if(!empty($post['dynamics'])){
        $dynamicsFields = $post['dynamics'];
        $dynamicsFields = array_filter($dynamicsFields);

        $typeofFields = $post['typeof'];
        $typeofFields = array_filter($typeofFields);

        $callbackFields = $post['callback'];
        $callbackFields = array_filter($callbackFields);

        foreach($dynamicsFields as $key => $field){

          $update = array(
            'user_fieldname' => $field,
          );

          if(!empty($typeofFields[$key])){
            $update['user_fieldtype'] = $typeofFields[$key];
          }

          if(!empty($callbackFields[$key])){
            $update['user_fieldcallback'] = $callbackFields[$key];
          }

          db_update('msdynamics')
              ->fields($update)
              ->condition('msdynamics_fieldname', $key, '=')
              ->execute();
          $num_updated++;
        }
    }

    if($form_state['values']['primary_key']){
        variable_set('msdynamics_primary_key', $form_state['values']['primary_key']);
    }
    

    drupal_set_message(t($num_updated . ' field'.($num_updated == 1 ? '' : 's').' linked'));

}


function msdynamics_ui_config_indextest_submit($form, &$form_state){
    drupal_set_message(t('The configuration is being '.$form_state['values']['run'].'ed.'));

    drupal_goto('admin/config/msdynamics/connection', 
        array(
            'query' => array(
                'run' =>  $form_state['values']['run']
            )
        )
    );
}

/**
 * Process system_themes_admin_form form submissions.
 */
function msdynamics_ui_config_submit($form, &$form_state) {

  $vars = msdynamics_get_vars();

  foreach($vars as $key => $var){
    variable_set('msdynamics_'.$key, $form_state['values'][$key]);
  }
  
  drupal_set_message(t('The connection settings have been saved.'));

}


/**
 * HELPERS
 */

function msdynamics_ui_load_user_fields(){

    $return = array();

    $userTable = db_query("SHOW COLUMNS FROM {users}", array(), array(
      'fetch' => PDO::FETCH_ASSOC,
    ));

    $fieldsTable = db_query("SELECT field_name AS Field FROM {field_config_instance} WHERE entity_type = 'user'", array(), array(
      'fetch' => PDO::FETCH_ASSOC,
    ));

    foreach($userTable as $userTableValue){
        $return[] = $userTableValue;
    }

    if(!empty($fieldsTable)){
        foreach($fieldsTable as $fieldsTableValue){
            $return[] = $fieldsTableValue;
        }
    }


    return $return;


}



function msdynamics_ui_notavailable(){
    return '<div class="messages warning">Not availble until you enter the connection details</div>';
}




