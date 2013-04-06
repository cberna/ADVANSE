SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='TRADITIONAL,ALLOW_INVALID_DATES';

DROP SCHEMA IF EXISTS `advanse` ;
CREATE SCHEMA IF NOT EXISTS `advanse` DEFAULT CHARACTER SET latin1 ;
USE `advanse` ;

-- -----------------------------------------------------
-- Table `advanse`.`phase`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`phase` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`phase` (
  `phase_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `phase_name` VARCHAR(50) NOT NULL ,
  PRIMARY KEY (`phase_id`) )
ENGINE = InnoDB
AUTO_INCREMENT = 8
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`section`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`section` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`section` (
  `section_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `section_name` VARCHAR(45) NOT NULL ,
  `professor_name` VARCHAR(45) NOT NULL ,
  PRIMARY KEY (`section_id`) )
ENGINE = InnoDB
AUTO_INCREMENT = 4
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`user`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`user` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`user` (
  `user_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `user_name` VARCHAR(45) NOT NULL ,
  `password` VARCHAR(45) NOT NULL ,
  `section_id` INT(11) NULL DEFAULT NULL ,
  PRIMARY KEY (`user_id`) ,
  INDEX `user_section_fk` (`section_id` ASC) ,
  CONSTRAINT `user_section_fk`
    FOREIGN KEY (`section_id` )
    REFERENCES `advanse`.`section` (`section_id` )
    ON DELETE SET NULL
    ON UPDATE CASCADE)
ENGINE = InnoDB
AUTO_INCREMENT = 32
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`project`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`project` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`project` (
  `user_project_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `user_id` INT(11) NOT NULL ,
  `psp_id` INT(11) NOT NULL DEFAULT '1' ,
  `current_phase` INT(11) NOT NULL ,
  `done` TINYINT(1) NOT NULL DEFAULT '0' ,
  PRIMARY KEY (`user_project_id`) ,
  INDEX `project_user_fk` (`user_id` ASC) ,
  INDEX `project_phase_fk` (`current_phase` ASC) ,
  CONSTRAINT `project_phase_fk`
    FOREIGN KEY (`current_phase` )
    REFERENCES `advanse`.`phase` (`phase_id` )
    ON UPDATE CASCADE,
  CONSTRAINT `project_user_fk`
    FOREIGN KEY (`user_id` )
    REFERENCES `advanse`.`user` (`user_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
AUTO_INCREMENT = 39
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`plansummary`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`plansummary` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`plansummary` (
  `plan_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `user_project_id` INT(11) NOT NULL ,
  PRIMARY KEY (`plan_id`) ,
  INDEX `plansummary_user_fk` (`user_project_id` ASC) ,
  CONSTRAINT `plansummary_user_fk`
    FOREIGN KEY (`user_project_id` )
    REFERENCES `advanse`.`project` (`user_project_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`actualloc`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`actualloc` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`actualloc` (
  `actualLOC_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `plan_id` INT(11) NOT NULL ,
  `actualBaseLOC` INT(11) NOT NULL DEFAULT '0' ,
  `actualDeletedLOC` INT(11) NOT NULL DEFAULT '0' ,
  `actualModifiedLOC` INT(11) NOT NULL DEFAULT '0' ,
  `actualAddedLOC` INT(11) NOT NULL DEFAULT '0' ,
  `actualReusedLOC` INT(11) NOT NULL DEFAULT '0' ,
  PRIMARY KEY (`actualLOC_id`) ,
  INDEX `actualLOC_plan_fk` (`plan_id` ASC) ,
  CONSTRAINT `actualLOC_plan_fk`
    FOREIGN KEY (`plan_id` )
    REFERENCES `advanse`.`plansummary` (`plan_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`objecttype`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`objecttype` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`objecttype` (
  `type_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `type_name` VARCHAR(45) NOT NULL ,
  PRIMARY KEY (`type_id`) )
ENGINE = InnoDB
AUTO_INCREMENT = 7
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`sizetype`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`sizetype` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`sizetype` (
  `size_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `size_name` VARCHAR(45) NOT NULL ,
  PRIMARY KEY (`size_id`) )
ENGINE = InnoDB
AUTO_INCREMENT = 6
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`objectsize`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`objectsize` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`objectsize` (
  `objectsize_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `type_id` INT(11) NOT NULL ,
  `size_id` INT(11) NOT NULL ,
  `size` FLOAT NOT NULL ,
  PRIMARY KEY (`objectsize_id`) ,
  INDEX `type_objectsize_fk` (`type_id` ASC) ,
  INDEX `type_sizetype_fk` (`size_id` ASC) ,
  CONSTRAINT `type_objectsize_fk`
    FOREIGN KEY (`type_id` )
    REFERENCES `advanse`.`objecttype` (`type_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `type_sizetype_fk`
    FOREIGN KEY (`size_id` )
    REFERENCES `advanse`.`sizetype` (`size_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`sizeestimate`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`sizeestimate` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`sizeestimate` (
  `template_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `user_project_id` INT(11) NOT NULL ,
  PRIMARY KEY (`template_id`) ,
  INDEX `user_project_size_fk` (`user_project_id` ASC) ,
  CONSTRAINT `user_project_size_fk`
    FOREIGN KEY (`user_project_id` )
    REFERENCES `advanse`.`project` (`user_project_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`baseaddition`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`baseaddition` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`baseaddition` (
  `baseAddition_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `template_id` INT(11) NOT NULL ,
  `function_name` VARCHAR(100) NOT NULL ,
  `objectsize_id` INT(11) NOT NULL ,
  PRIMARY KEY (`baseAddition_id`) ,
  INDEX `template_base_fk` (`template_id` ASC) ,
  INDEX `objectsize_base_fk` (`objectsize_id` ASC) ,
  CONSTRAINT `objectsize_base_fk`
    FOREIGN KEY (`objectsize_id` )
    REFERENCES `advanse`.`objectsize` (`objectsize_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `template_base_fk`
    FOREIGN KEY (`template_id` )
    REFERENCES `advanse`.`sizeestimate` (`template_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`defecttype`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`defecttype` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`defecttype` (
  `type_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `type_name` VARCHAR(45) NOT NULL ,
  `type_desc` TEXT NULL DEFAULT NULL ,
  PRIMARY KEY (`type_id`) )
ENGINE = InnoDB
AUTO_INCREMENT = 11
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`defect`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`defect` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`defect` (
  `defect_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `user_project_id` INT(11) NOT NULL ,
  `inject_phase` INT(11) NULL DEFAULT NULL ,
  `fix_phase` INT(11) NULL DEFAULT NULL ,
  `start_fix_time` BIGINT(20) NOT NULL ,
  `end_fix_time` BIGINT(20) NOT NULL ,
  `description` TEXT NULL DEFAULT NULL ,
  `type_id` INT(11) NOT NULL ,
  PRIMARY KEY (`defect_id`) ,
  INDEX `user_project_defect_fk` (`user_project_id` ASC) ,
  INDEX `inject_phase_defect_fk` (`inject_phase` ASC) ,
  INDEX `defect_type_fk` (`type_id` ASC) ,
  CONSTRAINT `inject_phase_defect_fk`
    FOREIGN KEY (`inject_phase` )
    REFERENCES `advanse`.`phase` (`phase_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `defect_type_fk`
    FOREIGN KEY (`type_id` )
    REFERENCES `advanse`.`defecttype` (`type_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `user_project_defect_fk`
    FOREIGN KEY (`user_project_id` )
    REFERENCES `advanse`.`project` (`user_project_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`defectdependency`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`defectdependency` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`defectdependency` (
  `dependency_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `defect_created_id` INT(11) NOT NULL ,
  `defect_original_id` INT(11) NOT NULL ,
  PRIMARY KEY (`dependency_id`) ,
  INDEX `defect_fk` (`defect_created_id` ASC) ,
  INDEX `defect_original_fk` (`defect_original_id` ASC) ,
  CONSTRAINT `defect_fk`
    FOREIGN KEY (`defect_created_id` )
    REFERENCES `advanse`.`defect` (`defect_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `defect_original_fk`
    FOREIGN KEY (`defect_original_id` )
    REFERENCES `advanse`.`defect` (`defect_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`fixpause`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`fixpause` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`fixpause` (
  `pause_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `defect_id` INT(11) NOT NULL ,
  `start_time` BIGINT(20) NOT NULL DEFAULT '0' ,
  `end_time` BIGINT(20) NOT NULL DEFAULT '0' ,
  PRIMARY KEY (`pause_id`) ,
  INDEX `fix_interrupt_fk` (`defect_id` ASC) ,
  CONSTRAINT `fix_interrupt_fk`
    FOREIGN KEY (`defect_id` )
    REFERENCES `advanse`.`defect` (`defect_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`timelog`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`timelog` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`timelog` (
  `log_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `user_project_id` INT(11) NOT NULL ,
  `phase_id` INT(11) NOT NULL ,
  `start_time` BIGINT(20) NOT NULL DEFAULT '0' ,
  `end_time` BIGINT(20) NOT NULL DEFAULT '0' ,
  `remarks` TEXT NULL DEFAULT NULL ,
  PRIMARY KEY (`log_id`) ,
  INDEX `user_project_timelog_fk` (`user_project_id` ASC) ,
  INDEX `phase_timelog_fk` (`phase_id` ASC) ,
  CONSTRAINT `phase_timelog_fk`
    FOREIGN KEY (`phase_id` )
    REFERENCES `advanse`.`phase` (`phase_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `user_project_timelog_fk`
    FOREIGN KEY (`user_project_id` )
    REFERENCES `advanse`.`project` (`user_project_id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`interruption`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`interruption` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`interruption` (
  `interruption_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `log_id` INT(11) NOT NULL ,
  `start_time` BIGINT(20) NOT NULL DEFAULT '0' ,
  `end_time` BIGINT(20) NOT NULL DEFAULT '0' ,
  `reason` TEXT NULL DEFAULT NULL ,
  PRIMARY KEY (`interruption_id`) ,
  INDEX `log_id_interruption_fk` (`log_id` ASC) ,
  CONSTRAINT `log_id_interruption_fk`
    FOREIGN KEY (`log_id` )
    REFERENCES `advanse`.`timelog` (`log_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`newobject`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`newobject` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`newobject` (
  `newobject_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `template_id` INT(11) NOT NULL ,
  `function_name` VARCHAR(100) NOT NULL ,
  `objectsize_id` INT(11) NOT NULL ,
  PRIMARY KEY (`newobject_id`) ,
  INDEX `template_new_fk` (`template_id` ASC) ,
  INDEX `size_new_fk` (`objectsize_id` ASC) ,
  CONSTRAINT `size_new_fk`
    FOREIGN KEY (`objectsize_id` )
    REFERENCES `advanse`.`objectsize` (`objectsize_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE,
  CONSTRAINT `template_new_fk`
    FOREIGN KEY (`template_id` )
    REFERENCES `advanse`.`sizeestimate` (`template_id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`piplan`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`piplan` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`piplan` (
  `piplan_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `user_project_id` INT(11) NOT NULL ,
  `notes` TEXT NULL DEFAULT NULL ,
  PRIMARY KEY (`piplan_id`) ,
  INDEX `userproject_pip_fk` (`user_project_id` ASC) ,
  INDEX `user_project_piplan_fk` (`user_project_id` ASC) ,
  CONSTRAINT `user_project_piplan_fk`
    FOREIGN KEY (`user_project_id` )
    REFERENCES `advanse`.`project` (`user_project_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
AUTO_INCREMENT = 6
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`pipproblem`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`pipproblem` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`pipproblem` (
  `problem_id` INT(11) NOT NULL ,
  `piplan_id` INT(11) NOT NULL ,
  `description` TEXT NULL DEFAULT NULL ,
  PRIMARY KEY (`problem_id`, `piplan_id`) ,
  INDEX `piplan_pipproblem_fk` (`piplan_id` ASC) ,
  CONSTRAINT `piplan_pipproblem_fk`
    FOREIGN KEY (`piplan_id` )
    REFERENCES `advanse`.`piplan` (`piplan_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`pipproposal`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`pipproposal` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`pipproposal` (
  `proposal_id` INT(11) NOT NULL ,
  `piplan_id` INT(11) NOT NULL ,
  `description` TEXT NULL DEFAULT NULL ,
  PRIMARY KEY (`proposal_id`, `piplan_id`) ,
  INDEX `piplan_proposal_fk` (`piplan_id` ASC) ,
  CONSTRAINT `piplan_proposal_fk`
    FOREIGN KEY (`piplan_id` )
    REFERENCES `advanse`.`piplan` (`piplan_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`planinfo`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`planinfo` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`planinfo` (
  `planinfo_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `plan_id` INT(11) NOT NULL ,
  `planLOCPerHour` INT(11) NOT NULL DEFAULT '0' ,
  `planTotalTime` INT(11) NOT NULL DEFAULT '0' ,
  `planPercentReused` INT(11) NOT NULL DEFAULT '0' ,
  `planPercentNewReused` INT(11) NOT NULL DEFAULT '0' ,
  `actualPercentReused` INT(11) NOT NULL DEFAULT '0' ,
  `actualPercentNewReused` INT(11) NOT NULL DEFAULT '0' ,
  PRIMARY KEY (`planinfo_id`) ,
  INDEX `planinfo_plan_fk` (`plan_id` ASC) ,
  CONSTRAINT `planinfo_plan_fk`
    FOREIGN KEY (`plan_id` )
    REFERENCES `advanse`.`plansummary` (`plan_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`plannedloc`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`plannedloc` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`plannedloc` (
  `plannedLOC_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `plan_id` INT(11) NOT NULL ,
  `planBaseLOC` FLOAT NOT NULL DEFAULT '0' ,
  `planDeletedLOC` FLOAT NOT NULL DEFAULT '0' ,
  `planModifiedLOC` FLOAT NOT NULL DEFAULT '0' ,
  `planAddedLOC` FLOAT NOT NULL DEFAULT '0' ,
  `planReusedLOC` FLOAT NOT NULL DEFAULT '0' ,
  PRIMARY KEY (`plannedLOC_id`) ,
  INDEX `plannedLOC_plan_FK` (`plan_id` ASC) ,
  CONSTRAINT `plannedLOC_plan_FK`
    FOREIGN KEY (`plan_id` )
    REFERENCES `advanse`.`plansummary` (`plan_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`plannedtime`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`plannedtime` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`plannedtime` (
  `plannedtime_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `plan_id` INT(11) NOT NULL ,
  `planningTime` INT(11) NOT NULL DEFAULT '0' ,
  `designTime` INT(11) NOT NULL DEFAULT '0' ,
  `codeTime` INT(11) NOT NULL DEFAULT '0' ,
  `compileTime` INT(11) NOT NULL DEFAULT '0' ,
  `testTime` INT(11) NOT NULL DEFAULT '0' ,
  `postMortemTime` INT(11) NOT NULL DEFAULT '0' ,
  PRIMARY KEY (`plannedtime_id`) ,
  INDEX `planTime_plan_fk` (`plan_id` ASC) ,
  CONSTRAINT `planTime_plan_fk`
    FOREIGN KEY (`plan_id` )
    REFERENCES `advanse`.`plansummary` (`plan_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`reusedobject`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`reusedobject` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`reusedobject` (
  `reusedobject_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `template_id` INT(11) NOT NULL ,
  `function_name` VARCHAR(100) NOT NULL ,
  `function_lines` FLOAT NOT NULL DEFAULT '0' ,
  PRIMARY KEY (`reusedobject_id`) ,
  INDEX `template_reused_fk` (`template_id` ASC) ,
  CONSTRAINT `template_reused_fk`
    FOREIGN KEY (`template_id` )
    REFERENCES `advanse`.`sizeestimate` (`template_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`taskplan`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`taskplan` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`taskplan` (
  `taskplan_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `user_project_id` INT(11) NOT NULL ,
  PRIMARY KEY (`taskplan_id`) ,
  INDEX `taskplan_user_fk` (`user_project_id` ASC) ,
  CONSTRAINT `taskplan_user_fk`
    FOREIGN KEY (`user_project_id` )
    REFERENCES `advanse`.`project` (`user_project_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`taskplanentry`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`taskplanentry` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`taskplanentry` (
  `entry_id` INT(11) NOT NULL AUTO_INCREMENT ,
  `taskplan_id` INT(11) NOT NULL ,
  `phase_id` INT(11) NOT NULL ,
  `date_planned` BIGINT(20) NOT NULL ,
  `date_accomplished` BIGINT(20) NOT NULL DEFAULT '0' ,
  `taskplanentrycol` VARCHAR(45) NULL DEFAULT '0' ,
  PRIMARY KEY (`entry_id`) ,
  INDEX `taskplan_entry_fk` (`taskplan_id` ASC) ,
  INDEX `phase_entry_fk` (`phase_id` ASC) ,
  CONSTRAINT `phase_entry_fk`
    FOREIGN KEY (`phase_id` )
    REFERENCES `advanse`.`phase` (`phase_id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `taskplan_entry_fk`
    FOREIGN KEY (`taskplan_id` )
    REFERENCES `advanse`.`taskplan` (`taskplan_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;


-- -----------------------------------------------------
-- Table `advanse`.`testreport`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `advanse`.`testreport` ;

CREATE  TABLE IF NOT EXISTS `advanse`.`testreport` (
  `test_number` INT(11) NOT NULL ,
  `user_project_id` INT(11) NOT NULL ,
  `test_objective` TEXT NULL DEFAULT NULL ,
  `test_conditions` TEXT NULL DEFAULT NULL ,
  `expected_results` TEXT NULL DEFAULT NULL ,
  `actual_results` TEXT NULL DEFAULT NULL ,
  `test_description` TEXT NULL DEFAULT NULL ,
  PRIMARY KEY (`test_number`, `user_project_id`) ,
  INDEX `user_project_testreport_fk` (`user_project_id` ASC) ,
  CONSTRAINT `user_project_testreport_fk`
    FOREIGN KEY (`user_project_id` )
    REFERENCES `advanse`.`project` (`user_project_id` )
    ON DELETE CASCADE
    ON UPDATE CASCADE)
ENGINE = InnoDB
DEFAULT CHARACTER SET = latin1;

USE `advanse` ;


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
