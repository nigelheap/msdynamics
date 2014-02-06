<div id="msdynamics_config">
    <div class="col-section">
        <?php echo (!empty($form) ? $form : ''); ?>
    </div>
    <div class="col-section">

        <?php echo (!empty($tester) ? $tester : ''); ?>
        
        <?php echo (!empty($index) ? $index : ''); ?>

        <div class="section-results">
            <?php echo (!empty($tester_result) ? '<pre>' . $tester_result . '</pre>' : ''); ?>
            <?php echo (!empty($index_result) ? '<pre>' . $index_result . '</pre>' : ''); ?>
            <?php if(empty($tester_result) && empty($index_result)): ?>
                <p>Test and indexing results will be displayed here</p>

            <?php endif; ?>
        </div>

    </div>

</div>





